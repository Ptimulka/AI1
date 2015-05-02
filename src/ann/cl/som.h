#pragma once

#include "typedefs.h"

namespace cl
{
    class SOM
    {
    public:
        SOM(uint lattice_width, uint lattice_height, uint input_dim);
        ~SOM();

        SOM(SOM const& s) = delete;
        SOM(SOM&& s) = delete;



    private:
        uint _owidth = 0;
        uint _oheight = 0;
        uint _idim = 0;
    };
}
