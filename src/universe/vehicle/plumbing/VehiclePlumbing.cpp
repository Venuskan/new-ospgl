#include "VehiclePlumbing.h"
#include "../Vehicle.h"

// A reasonable multiplier to prevent extreme flow velocities
// I don't know enough fluid mechanics as to determine a reasonable value
// so it's arbitrary, chosen to approximate real life rocket values
#define FLOW_MULTIPLIER 0.000002


VehiclePlumbing::VehiclePlumbing(Vehicle *in_vehicle)
{
	veh = in_vehicle;
}

std::vector<PlumbingMachine*> VehiclePlumbing::grid_aabb_check(glm::vec2 start, glm::vec2 end,
															   const std::vector<PlumbingMachine*>& ignore, bool expand)
{
	std::vector<PlumbingMachine*> out;
	std::vector<PlumbingMachine*> all_elems = get_all_elements();

	for(PlumbingMachine* pb : all_elems)
	{
		bool ignored = false;
		for(PlumbingMachine* m : ignore)
		{
			if(m == pb)
			{
				ignored = true;
				break;
			}
		}

		if(!ignored)
		{
			glm::ivec2 min = pb->editor_position;
			glm::ivec2 max = min + pb->get_size(expand);

			if (min.x < end.x && max.x > start.x && min.y < end.y && max.y > start.y)
			{
				out.emplace_back(pb);
			}
		}
	}


	return out;
}

glm::ivec4 VehiclePlumbing::get_plumbing_bounds()
{
	glm::ivec2 amin = glm::ivec2(INT_MAX, INT_MAX);
	glm::ivec2 amax = glm::ivec2(INT_MIN, INT_MIN);

	std::vector<PlumbingMachine*> all_elems = get_all_elements();
	bool any = false;
	for (PlumbingMachine* pb : all_elems)
	{
		glm::ivec2 min = pb->editor_position;
		glm::ivec2 max = pb->get_size(true);

		amin = glm::min(amin, min);
		amax = glm::max(amax, max);
		any = true;
	}

	if(any)
	{
		return glm::ivec4(amin.x, amin.y, amax.x - amin.x, amax.y - amin.y);
	}
	else
	{
		return glm::ivec4(0, 0, 0, 0);
	}
}

glm::ivec2 VehiclePlumbing::find_free_space(glm::ivec2 size)
{
	// We first do a binary search in the currently used space

	// If that cannot be found, return outside of used space, to the
	// bottom as rockets are usually vertical
	glm::ivec4 bounds = get_plumbing_bounds();
	return glm::ivec2(bounds.x, bounds.y + bounds.w);


}

glm::ivec2 VehiclePlumbing::get_plumbing_size_of(Part* p)
{
	// min is always (0, 0) as that's the forced origin of the plumbing start positions
	glm::ivec2 max = glm::ivec2(INT_MIN, INT_MIN);
	bool found_any = false;

	for(const auto& pair : p->machines)
	{
		PlumbingMachine& pb = pair.second->plumbing;
		if(pb.has_lua_plumbing())
		{
			found_any = true;
			max = glm::max(max, pb.editor_position + pb.get_size());
		}
	}

	if(found_any)
	{
		return max;
	}
	else
	{
		return glm::ivec2(0, 0);
	}

}

std::vector<PlumbingMachine*> VehiclePlumbing::get_all_elements()
{
	std::vector<PlumbingMachine*> out;

	// Machines
	for(const Part* p : veh->parts)
	{
		for (const auto &pair : p->machines)
		{
			if (pair.second->plumbing.has_lua_plumbing())
			{
				// Safe to take a pointer, parts are in the heap
				out.push_back(&pair.second->plumbing);
			}
		}

		for(const auto &amac: p->attached_machines)
		{
			if(amac->plumbing.has_lua_plumbing())
			{
				out.push_back(&amac->plumbing);
			}
		}
	}

	return out;
}

int VehiclePlumbing::create_pipe()
{
	Pipe p = Pipe();
	pipes.push_back(p);
	return pipes.size() - 1;
}

void VehiclePlumbing::execute_flows(float dt, std::vector<FlowPath>& flows)
{
	for(const FlowPath& path : flows)
	{
		// The real machine discards into the next (must be flow machine, except the last)
		// This way we implement fluid movement and not just teletransportation
		// (This allows buffer pipes, etc...)
		float to_move = -path.delta_P * FLOW_MULTIPLIER * dt;
		if(to_move == 0.0f)
			continue;
		StoredFluids buffer;
		Pipe& start = pipes[path.path[0]];
		Pipe& end = pipes[path.path.back()];
		// Bleed into the buffer
		FluidPort* from = path.backwards ? start.b : start.a;
		FluidPort* to = path.backwards ? end.a : end.b;

		logger->check(!from->is_flow_port && !to->is_flow_port, "A path starts/ends in a flow machine!");
		buffer.modify(from->in_machine->out_flow(from->id, to_move, true));
		float flow = buffer.get_total_gas_mass() + buffer.get_total_liquid_mass();
		start.flow += path.backwards ? flow : -flow;
		end.flow += path.backwards ? flow : -flow;

		// Travel the path if there's one
		if(path.path.size() > 1)
		{
			for (size_t i = 1; i < path.path.size() - 1; i++)
			{
				Pipe &p = pipes[path.path[i]];
				p.flow += path.backwards ? flow : -flow;
				FluidPort* from_p = path.backwards ? p.b : p.a;
				FluidPort* to_p = path.backwards ? p.a : p.b;
				logger->check(from_p->is_flow_port && to_p->is_flow_port, "True machine in middle of path!");
				// TODO!
			}
		}

		to->in_machine->in_flow(to->id, buffer, true);

	}

}


void VehiclePlumbing::find_all_possible_paths(std::vector<FlowPath> &fws)
{
	// Find a new unvisited pipe to start from that's connected to a real port
	for(size_t pipe_id = 0; pipe_id < pipes.size(); pipe_id++)
	{
		Pipe& pipe = pipes[pipe_id];

		for (const FluidPort &pa: pipe.a->in_machine->fluid_ports)
		{
			if (!pa.is_flow_port)
			{
				find_all_possible_paths_from(fws, pipe_id, false);
				break;
			}
		}

		for (const FluidPort &pb: pipe.b->in_machine->fluid_ports)
		{
			if (!pb.is_flow_port)
			{
				find_all_possible_paths_from(fws, pipe_id, true);
				break;
			}
		}

	}

}

void VehiclePlumbing::find_all_possible_paths_from(std::vector<FlowPath> &fws, size_t start, bool backwards)
{
	// start.a/b contains a true port, so we just need to travel to port b/a now,
	// this is a tree search algorithm
	// We store the whole path to the open pipe so we can rebuild it later
	std::queue<std::vector<size_t>> open;
	std::vector<size_t> start_v;
	start_v.push_back(start);
	open.push(start_v);

	while(!open.empty())
	{
		std::vector<size_t> open_chain = open.front();
		size_t work = open_chain.back();
		const Pipe* p = &pipes[work];
		const FluidPort* next = backwards ? p->a : p->b;

		open.pop();

		if(next->is_flow_port)
		{
			// Find all connected ports to this port and propagate in correct direction
			std::vector<FluidPort*> connected = next->in_machine->get_connected_ports(next->id);
			// Find the pipes connected as "a" to said ports, others are included in symmetric searches
			for(size_t pipe_id = 0; pipe_id < pipes.size(); pipe_id++)
			{
				Pipe& pipe = pipes[pipe_id];
				for(const FluidPort* port : connected)
				{
					if(backwards ? pipe.b == port : pipe.a == port)
					{
						std::vector<size_t> new_open;
						new_open.insert(new_open.begin(), open_chain.begin(), open_chain.end());
						new_open.push_back(pipe_id);
						open.push(new_open);
					}
				}
			}
		}
		else
		{
			FlowPath fpath;
			fpath.path = open_chain;
			fpath.backwards = backwards;
			calculate_delta_p(fpath, backwards);
			if(fpath.delta_P < 0.0f)
			{
				fws.push_back(fpath);
			}

		}
	}


}

void VehiclePlumbing::calculate_delta_p(FlowPath& fws, bool backwards)
{
	// It's guaranteed that all b ports are flow machines, so just follow them (except the end one)
	FluidPort* start = backwards ? pipes[fws.path[0]].b : pipes[fws.path[0]].a;
	float start_P = start->in_machine->get_pressure(start->id);
	FluidPort* end = backwards ? pipes[fws.path[fws.path.size() - 1]].a : pipes[fws.path[fws.path.size() - 1]].b;
	float end_P = end->in_machine->get_pressure(end->id);

	float P_drop = 0.0f;
	// We travel through the flow machines, observing the pressure drop between their ports
	// Note that the pressure drops happen to the start pressure (observe delta_P calculation)!
	for(size_t i = 0; i < fws.path.size() - 1; i++)
	{
		float cur_P = start_P - P_drop;
		FluidPort* in_port = pipes[fws.path[i]].b;
		FluidPort* out_port = pipes[fws.path[i + 1]].a;
		if(backwards)
		{
			std::swap(in_port, out_port);
		}

		logger->check(in_port->in_machine == out_port->in_machine, "Something went wrong, machines don't match");
		logger->check(in_port->is_flow_port && out_port->is_flow_port, "Flow ports mismatched!");
		P_drop += in_port->in_machine->get_pressure_drop(in_port->id, out_port->id, cur_P);
	}

	fws.delta_P = end_P - (start_P - P_drop);
	// If this is 0 or positive, it will be early discarded
	// End pressure must be lower than start pressure!
}

std::vector<size_t> VehiclePlumbing::find_forced_paths(std::vector<FlowPath>& fws)
{
	std::vector<size_t> out;
	// We must go over every path and check which paths start and end in unique places, these
	// are the forced paths. The places are the port!
	std::set<FluidPort*> seen_start;
	std::set<FluidPort*> seen_end;

	std::set<FluidPort*> multiple_start;
	std::set<FluidPort*> multiple_end;

	for(const FlowPath& path : fws)
	{
		Pipe* front = &pipes[path.path.front()];
		Pipe* back = &pipes[path.path.back()];
		logger->check(front->a->is_flow_port == false, "Start must not be flow port");
		logger->check(front->b->is_flow_port == false, "End must not be flow port");

		if(seen_start.count(front->a) >= 1)
		{
			multiple_start.insert(front->a);
		}
		else
		{
			seen_start.insert(front->a);
		}

		if(seen_end.count(back->b) >= 1)
		{
			multiple_end.insert(back->b);
		}
		else
		{
			seen_end.insert(back->b);
		}
	}

	// Now, the elements which are not in multiple are unique
	for(size_t idx = 0; idx < fws.size(); idx++)
	{
		Pipe* front = &pipes[fws[idx].path.front()];
		Pipe* back = &pipes[fws[idx].path.back()];

		// If this is a unique path, it must happen and thus is forced.
		if(multiple_start.count(front->a) == 0 && multiple_end.count(back->b) == 0)
		{
			out.push_back(idx);
		}
	}

	return out;
}

bool VehiclePlumbing::remove_paths_not_compatible_with_forced(std::vector<FlowPath> &fws)
{
	std::vector<size_t> forced = find_forced_paths(fws);
	std::vector<size_t> to_remove;

	for(size_t i = 0; i < fws.size(); i++)
	{
		for(size_t f = 0; f < forced.size(); f++)
		{
			if(forced[f] != i)
			{
				if(!are_paths_compatible(fws[i], fws[forced[f]]))
				{
					to_remove.push_back(i);
				}
			}
		}
	}

	// Now we remove the given indices
	vector_remove_indices(fws, to_remove);

	return to_remove.empty();

}

bool VehiclePlumbing::are_paths_compatible(const VehiclePlumbing::FlowPath &a, const VehiclePlumbing::FlowPath &b)
{
	// We find shared points between the two paths
	for(size_t i = 0; i < a.path.size(); i++)
	{
		auto it = std::find(b.path.begin(), b.path.end(), a.path[i]);
		if(it != b.path.end())
		{
			// Check that the traverse direction is the same, ie, the next pipe is the same.
			// if it's not, we have found an incompatibility
			if((i == a.path.size() - 1 && it++ != b.path.end()) ||
					(i != a.path.size() - 1 && it++ == b.path.end()))
			{
				// Path a ends, but path b doesn't. It must mean they diverge
				// This prevents vector overflow
				return false;
			}

			// Now it's safe to check next index
			size_t next_a = a.path[i + 1];
			size_t next_b = *(it++);
			if(next_a != next_b)
			{
				return false;
			}
		}
	}

	return true;
}

void VehiclePlumbing::reduce_to_forced_paths(std::vector<FlowPath> &fws)
{
	size_t it = 0;
	// This removes all non compatible paths
	while(!remove_paths_not_compatible_with_forced(fws))
	{
		if(it > 100)
		{
			logger->fatal("Reducing to forced paths is taking too long!");
		}
		it++;
	}

	// Now only forced paths remain, we have solved the system

}

int VehiclePlumbing::find_pipe_connected_to(FluidPort *port)
{
	// This is only used during vehicle editor so perfomance is not critical
	for(size_t i = 0; i < pipes.size(); i++)
	{
		if(pipes[i].a == port || pipes[i].b == port)
		{
			return i;
		}
	}

	return -1;
}

void VehiclePlumbing::update_pipes(float dt, Vehicle *in_vehicle)
{
	std::vector<FlowPath> paths;
	// Clear flows in pipes
	for(Pipe& p : pipes)
	{
		p.flow = 0.0f;
	}
	find_all_possible_paths(paths);
	reduce_to_forced_paths(paths);
	execute_flows(dt, paths);

}

void VehiclePlumbing::init()
{
	for(Pipe& p : pipes)
	{
		if(p.amachine)
		{
			p.a = p.amachine->get_port_by_id(p.aport);
			p.amachine = nullptr;
			p.aport = "";
		}

		if(p.bmachine)
		{
			p.b = p.bmachine->get_port_by_id(p.bport);
			p.bmachine = nullptr;
			p.bport = "";
		}
	}
}


void Pipe::invert()
{
	std::swap(a, b);
	std::reverse(waypoints.begin(), waypoints.end());
}

Pipe::Pipe()
{
	a = nullptr;
	b = nullptr;
	surface = 1.0f;
	flow = 0.0f;

}
