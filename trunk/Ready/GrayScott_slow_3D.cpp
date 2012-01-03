/*  Copyright 2011, The Ready Bunch

    This file is part of Ready.

    Ready is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ready is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Ready. If not, see <http://www.gnu.org/licenses/>.         */

// local:
#include "GrayScott_slow_3D.hpp"
#include "utils.hpp"

// stdlib:
#include <stdlib.h>
#include <math.h>

// STL:
#include <stdexcept>
#include <cassert>
using namespace std;

// VTK:
#include <vtkImageData.h>

GrayScott_slow_3D::GrayScott_slow_3D()
{
    this->timestep = 1.0f;
    this->r_a = 0.082f;
    this->r_b = 0.041f;
    // for spots:
    this->k = 0.064f;
    this->f = 0.035f;
    this->buffer_image = NULL;
}

GrayScott_slow_3D::~GrayScott_slow_3D()
{
    if(this->buffer_image)
        this->buffer_image->Delete();
}

void GrayScott_slow_3D::Allocate(int x,int y,int z)
{
    this->AllocateImage(x,y,z,2);
    this->buffer_image = AllocateVTKImage(x,y,z,2);
}

void GrayScott_slow_3D::Update(int n_steps)
{
    // take approximately n_steps
    for(int iStepPair=0;iStepPair<(n_steps+1)/2;iStepPair++)
    {
        for(int iStep=0;iStep<2;iStep++)
        {
            vtkImageData *from_image,*to_image;
            switch(iStep) {
                case 0: from_image = this->GetImage(); to_image = this->buffer_image; break;
                case 1: from_image = this->buffer_image; to_image = this->GetImage(); break;
            }

            const int X = from_image->GetDimensions()[0];
            const int Y = from_image->GetDimensions()[1];
            const int Z = from_image->GetDimensions()[2];
            const int NC = from_image->GetNumberOfScalarComponents();

            float* from_data = static_cast<float*>(from_image->GetScalarPointer());
            float* to_data = static_cast<float*>(to_image->GetScalarPointer());

            for(int x=0;x<X;x++)
            {
                int x_prev = (x-1+X)%X;
                int x_next = (x+1)%X;
                for(int y=0;y<Y;y++)
                {
                    int y_prev = (y-1+Y)%Y;
                    int y_next = (y+1)%Y;
                    for(int z=0;z<Z;z++)
                    {
                        int z_prev = (z-1+Z)%Z;
                        int z_next = (z+1)%Z;

                        float aval = *vtk_at(from_data,x,y,z,0,X,Y,NC);
                        float bval = *vtk_at(from_data,x,y,z,1,X,Y,NC);

                        // compute the Laplacians of a and b
                        // 7-point stencil:
                        float dda = *vtk_at(from_data,x,y_prev,z,0,X,Y,NC) +
                                    *vtk_at(from_data,x,y_next,z,0,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y,z,0,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y,z,0,X,Y,NC) +
                                    *vtk_at(from_data,x,y,z_prev,0,X,Y,NC) + 
                                    *vtk_at(from_data,x,y,z_next,0,X,Y,NC) +
                                    - 6*aval;
                        float ddb = *vtk_at(from_data,x,y_prev,z,1,X,Y,NC) +
                                    *vtk_at(from_data,x,y_next,z,1,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y,z,1,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y,z,1,X,Y,NC) +
                                    *vtk_at(from_data,x,y,z_prev,1,X,Y,NC) + 
                                    *vtk_at(from_data,x,y,z_next,1,X,Y,NC) +
                                    - 6*bval; 
                        /* // 19-point stencil
                        // Spotz,W.F. and G.F. Carey, 1996, A high-order compact formulation for the 3D Poisson equation
                        // Numerical Methods for Partial Differential Equations, 12, 235�243.
                        // www.cfdlab.ae.utexas.edu/labstaff/carey/GFC_Papers/Carey146.pdf
                        // (also contains a 27-point stencil if we need it)
                        // N.B. code doesn't behave anything like the 7-point stencil above. Have I done something wrong?
                        float dda = 2 * (*vtk_at(from_data,x,y_prev,z,0,X,Y,NC) +
                                         *vtk_at(from_data,x,y_next,z,0,X,Y,NC) +
                                         *vtk_at(from_data,x_prev,y,z,0,X,Y,NC) + 
                                         *vtk_at(from_data,x_next,y,z,0,X,Y,NC) +
                                         *vtk_at(from_data,x,y,z_prev,0,X,Y,NC) + 
                                         *vtk_at(from_data,x,y,z_next,0,X,Y,NC) ) +
     
                                    *vtk_at(from_data,x,y_prev,z_prev,0,X,Y,NC) +
                                    *vtk_at(from_data,x,y_next,z_prev,0,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y,z_prev,0,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y,z_prev,0,X,Y,NC) +

                                    *vtk_at(from_data,x,y_prev,z_next,0,X,Y,NC) +
                                    *vtk_at(from_data,x,y_next,z_next,0,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y,z_next,0,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y,z_next,0,X,Y,NC) +

                                    *vtk_at(from_data,x_prev,y_prev,z,0,X,Y,NC) +
                                    *vtk_at(from_data,x_next,y_prev,z,0,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y_next,z,0,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y_next,z,0,X,Y,NC)

                                    - 24*aval;

                        float ddb = 2 * (*vtk_at(from_data,x,y_prev,z,1,X,Y,NC) +
                                         *vtk_at(from_data,x,y_next,z,1,X,Y,NC) +
                                         *vtk_at(from_data,x_prev,y,z,1,X,Y,NC) + 
                                         *vtk_at(from_data,x_next,y,z,1,X,Y,NC) +
                                         *vtk_at(from_data,x,y,z_prev,1,X,Y,NC) + 
                                         *vtk_at(from_data,x,y,z_next,1,X,Y,NC) ) +
     
                                    *vtk_at(from_data,x,y_prev,z_prev,1,X,Y,NC) +
                                    *vtk_at(from_data,x,y_next,z_prev,1,X,Y,NC) +
                                    *vtk_at(from_data,x,y_prev,z_next,1,X,Y,NC) +
                                    *vtk_at(from_data,x,y_next,z_next,1,X,Y,NC) +

                                    *vtk_at(from_data,x_prev,y,z_prev,1,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y,z_prev,1,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y,z_next,1,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y,z_next,1,X,Y,NC) +

                                    *vtk_at(from_data,x_prev,y_prev,z,1,X,Y,NC) +
                                    *vtk_at(from_data,x_next,y_prev,z,1,X,Y,NC) +
                                    *vtk_at(from_data,x_prev,y_next,z,1,X,Y,NC) + 
                                    *vtk_at(from_data,x_next,y_next,z,1,X,Y,NC)

                                    - 24*bval;*/

                        // compute the new rate of change of a and b
                        float da = this->r_a * dda - aval*bval*bval + this->f*(1-aval);
                        float db = this->r_b * ddb + aval*bval*bval - (this->f+this->k)*bval;

                        // apply the change
                        *vtk_at(to_data,x,y,z,0,X,Y,NC) = aval + this->timestep * da;
                        *vtk_at(to_data,x,y,z,1,X,Y,NC) = bval + this->timestep * db;
                    }
                }
            }

        }
        this->timesteps_taken+=2;
    }
    this->image->Modified();
}

void GrayScott_slow_3D::InitWithBlobInCenter()
{
    vtkImageData *image = this->GetImage();
    assert(image);

    const int X = image->GetDimensions()[0];
    const int Y = image->GetDimensions()[1];
    const int Z = image->GetDimensions()[2];
    const int NC = image->GetNumberOfScalarComponents();

    float* data = static_cast<float*>(image->GetScalarPointer());

    for(int x=0;x<X;x++)
    {
        for(int y=0;y<Y;y++)
        {
            for(int z=0;z<Z;z++)
            {
                if(hypot3(x-X/2,(y-Y/2)/1.5,z-Z/2)<=frand(2.0f,5.0f)) // start with a uniform field with an approximate sphere in the middle
                {
                    *vtk_at(data,x,y,z,0,X,Y,NC) = 0.0f;
                    *vtk_at(data,x,y,z,1,X,Y,NC) = 1.0f;
                }
                else 
                {
                    *vtk_at(data,x,y,z,0,X,Y,NC) = 1.0f;
                    *vtk_at(data,x,y,z,1,X,Y,NC) = 0.0f;
                }
            }
        }
    }
    image->Modified();
    this->timesteps_taken = 0;
}
