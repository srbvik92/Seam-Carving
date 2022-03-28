#ifndef SEAM_H
#define SEAM_H
#include <vector>
#include <utility>
using namespace std;

class Seam
{
public:
	// Default constructor
	Seam(): cost(0){};
	vector< pair<pair<int, int>, double> > seam;
	double cost;
	pair<int,int> coordinates(int i)
	{
		return seam[i].first;
	}

	// Deletes the entry at an index
	void deletePixel(int i)
	{
		cost -= seam[i].second;
		seam.erase(seam.begin() + i);
	}

	// Add a pixel to the seam
	void add(int i, int j, double energy)
	{
		cost += energy;
		seam.push_back(make_pair(make_pair(i, j), energy));
	}
};

#endif