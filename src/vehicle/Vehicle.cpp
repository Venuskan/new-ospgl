#include "Vehicle.h"
#include <glm/glm.hpp>
#include "../util/DebugDrawer.h"
#include "../physics/glm/BulletGlmCompat.h"

using WeldedGroupCreation = std::pair<std::unordered_set<Piece*>, bool>;

struct PieceState
{
	btTransform transform;
	btVector3 linear;
	btVector3 angular;
};

static PieceState obtain_piece_state(Piece* piece)
{
	PieceState st;
	st.transform = piece->get_global_transform();
	st.linear = piece->get_linear_velocity();
	st.angular = piece->get_angular_velocity();
	return st;
}

static void add_to_welded_groups(std::vector<WeldedGroupCreation>& welded_groups, Piece* piece)
{
	bool found = false;
	// Find what welded group to add this to
	if (piece->welded)
	{
		for (size_t i = 0; i < welded_groups.size(); i++)
		{
			if (welded_groups[i].first.count(piece->attached_to) != 0)
			{
				welded_groups[i].first.insert(piece);
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		// Create the welded group, add the part, and, if this part is welded, add
		// whatever it's welded to (Note: individual parts get a lone welded group)
		welded_groups.push_back(std::make_pair(std::unordered_set<Piece*>(), false));
		welded_groups[welded_groups.size() - 1].first.insert(piece);
		if (piece->welded)
		{
			welded_groups[welded_groups.size() - 1].first.insert(piece->attached_to);
		}
	}
}

static std::vector<Piece*> extract_single_pieces(std::vector<WeldedGroupCreation>& welded_groups)
{
	std::vector<Piece*> single_pieces;

	// Find single piece (groups of only one piece)
	for (auto it = welded_groups.begin(); it != welded_groups.end();)
	{
		if (it->first.size() == 1)
		{
			Piece* piece = *it->first.begin();

			single_pieces.push_back(piece);
			// Remove welded colliders, if it had any
			if (piece->in_group != nullptr)
			{
				piece->rigid_body = nullptr;
				piece->motion_state = nullptr;
				piece->in_group = nullptr;
			}

			it = welded_groups.erase(it);
		}
		else
		{
			it++;
		}
	}

	return single_pieces;
}

static void remove_outdated_welded_groups(
	std::vector<WeldedGroup*>& welded, std::vector<WeldedGroupCreation>& welded_groups, btDynamicsWorld* world)
{
	for (auto it = welded.begin(); it != welded.end();)
	{
		WeldedGroup* wgroup = *it;

		bool found = false;
		// Find any same group, to be the same, the welded_group
		// must contain every piece in wgroup, and be the same size
		for (size_t j = 0; j < welded_groups.size(); j++)
		{
			int count = 0;
			for (size_t i = 0; i < wgroup->pieces.size(); i++)
			{
				count += (int)welded_groups[j].first.count(wgroup->pieces[i]);
			}

			if (count == wgroup->pieces.size())
			{
				found = true;
				welded_groups[j].second = true;
				break;
			}
		}

		if (!found)
		{
			// We remove the group as it is not present anymore
			world->removeRigidBody(wgroup->rigid_body);

			delete wgroup->motion_state;
			delete wgroup->rigid_body;
			delete wgroup;

			it = welded.erase(it);
		}
		else
		{
			it++;
		}
	}
}

static void create_new_welded_group(
	std::vector<WeldedGroup*>& welded, WeldedGroupCreation& wg, 
	std::unordered_map<Piece*, PieceState>& states_at_start, btDynamicsWorld* world)
{
	if (wg.second == false)
	{
		// Create a new WeldedGroup
		WeldedGroup* n_group = new WeldedGroup();
		n_group->pieces.reserve(wg.first.size());

		// Create collider
		btCompoundShape temp_collider = btCompoundShape();
		btCompoundShape* collider = new btCompoundShape();

		btTransform identity;
		identity.setIdentity();

		std::vector<btScalar> masses;
		btTransform principal;
		principal.setIdentity();

		double tot_mass = 0.0;

		for (Piece* p : wg.first)
		{
			p->welded_tform = states_at_start[p].transform;
			temp_collider.addChildShape(p->welded_tform, p->collider);
			masses.push_back(p->mass);
			tot_mass += p->mass;

			// TODO: Remove link from part
		}

		// Create rigidbody
		btVector3 local_inertia;

		temp_collider.calculatePrincipalAxisTransform(masses.data(), principal, local_inertia);

		btTransform principal_inverse = principal.inverse();

		for (int i = 0; i < temp_collider.getNumChildShapes(); i++)
		{
			collider->addChildShape(principal_inverse * temp_collider.getChildTransform(i),
				temp_collider.getChildShape(i));
		}


		//collider->calculateLocalInertia(tot_mass, local_inertia);

		btMotionState* motion_state = new btDefaultMotionState(principal);
		btRigidBody::btRigidBodyConstructionInfo info(tot_mass, motion_state, collider, local_inertia);
		btRigidBody* rigid_body = new btRigidBody(info);

		rigid_body->setActivationState(DISABLE_DEACTIVATION);

		world->addRigidBody(rigid_body);

		btVector3 total_impulse = btVector3(0.0, 0.0, 0.0);
		btVector3 total_angvel = btVector3(0.0, 0.0, 0.0);

		for (Piece* p : wg.first)
		{
			n_group->pieces.push_back(p);
			p->in_group = n_group;
			p->rigid_body = rigid_body;
			p->motion_state = motion_state;
			p->welded_tform = principal_inverse * p->welded_tform;

			rigid_body->applyCentralImpulse(states_at_start[p].linear * p->mass);
			total_angvel += states_at_start[p].angular;
		}

		// TODO: Is this stuff correct?
		total_angvel /= (btScalar)wg.first.size();

		rigid_body->setAngularVelocity(total_angvel);

		n_group->rigid_body = rigid_body;
		n_group->motion_state = motion_state;

		welded.push_back(n_group);
	}
}

static void create_piece_physics(Piece* piece, std::unordered_map<Piece*, PieceState>& states_at_start, btDynamicsWorld* world)
{
	// We must be a lone piece if we haven't been touched before
	btVector3 local_inertia;
	piece->collider->calculateLocalInertia(piece->mass, local_inertia);

	btMotionState* motion_state = new btDefaultMotionState(states_at_start[piece].transform);
	btRigidBody::btRigidBodyConstructionInfo info(piece->mass, motion_state, piece->collider, local_inertia);
	btRigidBody* rigid_body = new btRigidBody(info);

	rigid_body->setActivationState(DISABLE_DEACTIVATION);

	world->addRigidBody(rigid_body);

	piece->rigid_body = rigid_body;
	piece->motion_state = motion_state;

	// Apply old impulses
	rigid_body->applyImpulse(states_at_start[piece].linear * piece->mass, piece->get_relative_position());
	rigid_body->setAngularVelocity(states_at_start[piece].angular);
}

void Vehicle::build_physics(btDynamicsWorld* world)
{	

	// We need to create shared colliders for all welded 
	// groups, and individual colliders for every other piece
	// The bool is used later on to check if the group was already present
	
	std::vector<WeldedGroupCreation> welded_groups;

	// We need this as we will remove rigidbodies, and with them,
	// physics information. 
	std::unordered_map<Piece*, PieceState> states_at_start;

	for (Piece* piece : all_pieces)
	{
		states_at_start[piece] = obtain_piece_state(piece);

		add_to_welded_groups(welded_groups, piece);
	}

	std::vector<Piece*> single_pieces = extract_single_pieces(welded_groups);

	remove_outdated_welded_groups(welded, welded_groups, world);

	for (WeldedGroupCreation& wg : welded_groups)
	{
		create_new_welded_group(welded, wg, states_at_start, world);
	}

	for (Piece* piece : single_pieces)
	{
		if (piece->rigid_body == nullptr)
		{
			create_piece_physics(piece, states_at_start, world);
		}
	}


	// TODO: Create links between non-welded parts

}

std::vector<Vehicle*> Vehicle::handle_separation()
{
	std::vector<Vehicle*> n_vehicles;

	// Index 0 is always new root part
	std::vector<std::vector<Piece*>> separated_groups;
	for (Piece* piece : all_pieces)
	{
		// Root is always separated ;)
		if (piece != root)
		{
			separated_groups.push_back(std::vector<Piece*>());
			separated_groups[separated_groups.size() - 1].push_back(piece);
		}
	}

	// Find all parts connected to any part in the separated group

	bool done = false;

	while (!done)
	{
		int added = 0;

		for (Piece* piece : all_pieces)
		{

			for (std::vector<Piece*>& group : separated_groups)
			{
				for (Piece* p : group)
				{
					if (piece->attached_to == p)
					{
						group.push_back(piece);
						added++;
					}
				}
			}

		}

		if (added == 0)
		{
			done = true;
		}
	}

	// Remove from our vehicle
	for (std::vector<Piece*>& group : separated_groups)
	{
		for (Piece* p : group)
		{
			for (size_t i = 0; i < all_pieces.size(); i++)
			{
				if (all_pieces[i] == p)
				{
					all_pieces.erase(all_pieces.begin() + i);
				}
			}
		}
	}

	// Add to new vehicles
	for (size_t i = 0; i < separated_groups.size(); i++)
	{
		Vehicle* n_vehicle = new Vehicle();

		for (size_t j = 0; j < separated_groups[i].size(); j++)
		{
			n_vehicle->all_pieces.push_back(separated_groups[i][j]);
		}
		
		n_vehicle->root = separated_groups[i][0];

		n_vehicles.push_back(n_vehicle);
	}

	return n_vehicles;
}

void Vehicle::draw_debug()
{
	for (size_t i = 0; i < all_pieces.size(); i++)
	{
		glm::vec3 color = glm::vec3(0.7, 0.7, 0.7);

		Piece* p = all_pieces[i];
		Piece* link = p->attached_to;

		if (link == nullptr)
		{
			color = glm::vec3(1.0, 0.7, 1.0);
		}

		glm::dvec3 ppos = to_dvec3(p->get_global_transform().getOrigin());

		debug_drawer->add_point(ppos, color);
		
		if (link != nullptr)
		{
			glm::dvec3 dpos = to_dvec3(link->get_global_transform().getOrigin());

			if (p->welded)
			{
				debug_drawer->add_line(ppos, dpos, glm::dvec3(0.8, 0.8, 0.8));
			}
			else
			{
				debug_drawer->add_line(ppos, dpos, glm::dvec3(0.8, 0.3, 0.3));
			}
		}
	}
}

Vehicle::Vehicle()
{
}


Vehicle::~Vehicle()
{
}
