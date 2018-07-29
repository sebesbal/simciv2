#include "indgen.h"

void indgen::Economy::optimize_positions()
{
	double delta = 1000;
	const double epsilon = 0.1;
	const double max_move = 5;

	double last_fitness = - transport_cost();
	while (delta > epsilon)
	{
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

		double new_fitness = - transport_cost();
		delta = new_fitness - last_fitness;
		last_fitness = new_fitness;
	}
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
