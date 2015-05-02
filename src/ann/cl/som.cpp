#include "som.h"
using namespace cl;

SOM::SOM(uint lattice_width, uint lattice_height, uint input_dim) : _owidth(lattice_width), _oheight(lattice_height), _idim(input_dim)
{
}

SOM::~SOM()
{
}
