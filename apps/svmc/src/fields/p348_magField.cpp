/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// # include "config.h"
# include <G4MagneticField.hh>
# include <G4ThreeVector.hh>
# include <G4SystemOfUnits.hh>
# include <G4Field.hh>

# include "g4extras/FieldDict.hpp"

namespace p348 {

class MagneticField : public G4MagneticField {

public:
    MagneticField();
    MagneticField(const G4ThreeVector & fieldValue);

    virtual ~MagneticField();

    virtual void GetFieldValue(const G4double [4], double* bField) const override;

    G4ThreeVector field_value() const { return _fieldValue; }
    void          field_value(const G4ThreeVector & fieldValue)
                    { _fieldValue = fieldValue; }

protected :
    G4ThreeVector _fieldValue;
};       // class MagneticField

// IMPLEMENTATION

MagneticField::MagneticField()
    : G4MagneticField(), _fieldValue(0., 0., 0.)
{}

MagneticField::MagneticField(const G4ThreeVector & fieldValue)
    : G4MagneticField(), _fieldValue(fieldValue)
{}


MagneticField::~MagneticField()
{}

void MagneticField::GetFieldValue(const G4double [4], double* bField) const
{
    bField[0] = _fieldValue[0]*tesla;
    bField[1] = _fieldValue[1]*tesla;
    bField[2] = _fieldValue[2]*tesla;
}

P348_G4_REGISTER_FIELD ( MagneticField )

}  // namespace p348


