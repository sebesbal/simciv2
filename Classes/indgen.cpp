#include "stdafx.h"
#include "indgen.h"

void indgen::Economy::optimize_positions()
{
	while (!optimize_positions_step());
}

bool indgen::Economy::optimize_positions_step()
{
	if (fitness == 0)
	{
		fitness = -transport_cost();
	}

	const double epsilon = 0.1;
	const double max_move = 5;

	for (auto& f : factories)
	{
		vec_t force(0, 0);
		for (auto& t : f.transports)
		{
			force += (t.b->pos - t.a->pos) * t.cost;
		}

		double length = force.size();
		if (length > max_move)
		{
			force *= max_move / length;
		}

		f.pos += force;
	}

	double new_fitness = -transport_cost();
	double delta = new_fitness - fitness;
	fitness = new_fitness;
	return delta < epsilon;
}

double indgen::Economy::transport_cost()
{
	double result = 0;
	for (auto& f : factories)
	{
		for (auto& t : f.transports)
		{
			result += t.cost;
		}
	}
	return result;
}
