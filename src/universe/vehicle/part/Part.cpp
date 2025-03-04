#include "Part.h"
#include <string>

Part::Part(AssetHandle<PartPrototype>& part_proto, std::shared_ptr<cpptoml::table> our_table)
{
	this->part_proto = part_proto.duplicate();
	this->our_table = our_table;

	// Pre-load machines (everything but lua)
	for(auto machine_toml : part_proto->machines)
	{
		std::string id = *machine_toml->get_as<std::string>("id");
		std::string script = *machine_toml->get_as<std::string>("script");

		// We must make a clone! Otherwise it modifies all instances of the machine
		auto config_toml = machine_toml->clone()->as_table();
		if(our_table)
		{
			auto override_toml = our_table->get_table(id);
			if (override_toml)
			{
				//std::cout << "Before: " << *config_toml << std::endl;
				SerializeUtil::override(*config_toml, *override_toml);
				//std::cout << "After: " << *config_toml << std::endl;
			}
		}

		std::string cur_pkg = part_proto.pkg;

		Machine* n_machine = new Machine(config_toml, cur_pkg);
		machines[id] = n_machine;

	}

}

void Part::pre_update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->pre_update(dt);
	}
	for(auto* machine : attached_machines)
	{
		machine->pre_update(dt);
	}
}

void Part::update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->update(dt);
	}
	for(auto* machine : attached_machines)
	{
		machine->update(dt);
	}
}

void Part::editor_update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->editor_update(dt);
	}

	for(auto* machine : attached_machines)
	{
		machine->editor_update(dt);
	}
}

void Part::physics_update(double dt)
{
	for(auto& machine_pair : machines)
	{
		machine_pair.second->physics_update(dt);
	}
	for(auto* machine : attached_machines)
	{
		machine->physics_update(dt);
	}
}


void Part::init(sol::state* st, Vehicle* in_vehicle)
{
	this->vehicle = in_vehicle;

	// Load machines
	for(auto& machine_pair : machines)
	{
		machine_pair.second->init(st, this);
	}

	for(auto* machine : attached_machines)
	{
		machine->init(st, this);
	}

}

Piece* Part::get_piece(const std::string& name) const
{
	auto it = pieces.find(name);
	logger->check(it != pieces.end(), "Tried to get a piece which does not exist on the part ({})", name);
	return it->second;
}

Machine* Part::get_machine(const std::string& id) const
{
	auto all_machines = get_all_machines();
	auto it = all_machines.find(id);
	logger->check(it != all_machines.end(), "Invalid machine ID '{}'", id);
	return it->second;
}


std::vector<Piece*> Part::create_pieces() 
{
	std::vector<Piece*> n_pieces;
	std::unordered_map<Piece*, std::string> piece_to_name;
	std::unordered_map<std::string, Piece*> name_to_piece;

	for(auto pair : part_proto->pieces)
	{
		Piece* p = new Piece(this, pair.first);
		// TODO: Check this
		p->packed_tform = p->piece_prototype->collider_offset;
		p->in_vehicle = vehicle;
		pieces[pair.first] = p;
		n_pieces.push_back(p);
		piece_to_name[p] = pair.first;
		name_to_piece[pair.first] = p;
	}			

	// We create all the needed links now
	for(Piece* p : n_pieces)
	{
		if(piece_to_name[p] != PartPrototype::ROOT_NAME)
		{
			p->attached_to =  name_to_piece[p->piece_prototype->attached_to];
			p->welded = p->piece_prototype->welded;
			// TODO: Load the link if present
			p->from_attachment = p->piece_prototype->from_attachment;
			p->to_attachment = p->piece_prototype->to_attachment;
			auto from_attch = p->find_attachment(p->from_attachment);
			auto to_attch = p->attached_to->find_attachment(p->to_attachment);

			from_attch ? from_attch->second = true : 0;
			to_attch ? to_attch->second = true : 0;			
		}
	}

	return n_pieces;
}

Part::~Part()
{
	for(auto& machine_pair : machines)
	{
		delete machine_pair.second;
	}

	for(auto* machine : attached_machines)
	{
		delete machine;
	}
}

std::unordered_map<std::string, Machine*> Part::get_all_machines() const
{
	std::unordered_map<std::string, Machine*> out;

	for(auto& machine_pair : machines)
	{
		out[machine_pair.first] = machine_pair.second;
	}

	size_t i = 0;
	for(auto* machine : attached_machines)
	{
		std::string str = "_attached_";
		str += std::to_string(i);
		out[str] = machine;
		i++;
	}

	return out;
}

