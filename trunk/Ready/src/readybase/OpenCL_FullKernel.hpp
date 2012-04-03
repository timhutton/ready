/*  Copyright 2011, 2012 The Ready Bunch

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
#include "OpenCL_RD.hpp"

/// n-dimensional (1D,2D,3D) OpenCL RD implementations with n chemicals
/** specified as a full OpenCL kernel, for maximum flexibility */
class OpenCL_FullKernel : public OpenCL_RD
{
    public:

        OpenCL_FullKernel();

        virtual void InitializeFromXML(vtkXMLDataElement* rd,bool& warn_to_update);
        virtual vtkSmartPointer<vtkXMLDataElement> GetAsXML() const;

        std::string GetRuleType() const { return "kernel"; }

        virtual bool HasEditableBlockSize() const { return true; }
        virtual int GetBlockSizeX() const { return this->block_size[0]; }
        virtual int GetBlockSizeY() const { return this->block_size[1]; }
        virtual int GetBlockSizeZ() const { return this->block_size[2]; }
        virtual void SetBlockSizeX(int n) { this->block_size[0]=n; this->need_reload_formula=true; }
        virtual void SetBlockSizeY(int n) { this->block_size[1]=n; this->need_reload_formula=true; }
        virtual void SetBlockSizeZ(int n) { this->block_size[2]=n; this->need_reload_formula=true; }

        virtual std::string AssembleKernelSourceFromFormula(std::string formula) const;
        
    protected:
    
        int block_size[3];
};
