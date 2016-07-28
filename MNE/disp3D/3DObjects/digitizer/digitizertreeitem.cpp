//=============================================================================================================
/**
* @file     digitizertreeitem.cpp
* @author   Jana Kiesel <jana.kiesel@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     July, 2016
*
* @section  LICENSE
*
* Copyright (C) 2016, Jana Kiesel and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    DigitizerTreeItem class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "digitizertreeitem.h"
#include "../../helpers/renderable3Dentity.h"

#include <fiff/fiff_constants.h>
#include <fiff/fiff_dig_point.h>


//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================

#include <QMatrix4x4>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QEntity>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace DISP3DLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

DigitizerTreeItem::DigitizerTreeItem(int iType, const QString& text)
: AbstractTreeItem(iType, text)
, m_pParentEntity(new Qt3DCore::QEntity())
, m_pRenderable3DEntity(new Renderable3DEntity())
{
    this->setEditable(false);
    this->setCheckable(true);
    this->setCheckState(Qt::Checked);
    this->setToolTip(text);
}


//*************************************************************************************************************

DigitizerTreeItem::~DigitizerTreeItem()
{
}


//*************************************************************************************************************

QVariant DigitizerTreeItem::data(int role) const
{
    return AbstractTreeItem::data(role);
}


//*************************************************************************************************************

void  DigitizerTreeItem::setData(const QVariant& value, int role)
{
    AbstractTreeItem::setData(value, role);

    switch(role) {
    case Data3DTreeModelItemRoles::SurfaceCurrentColorVert:
        m_pRenderable3DEntity->setVertColor(value.value<QByteArray>());
        break;
    }
}


//*************************************************************************************************************

bool DigitizerTreeItem::addData(const QList<FIFFLIB::FiffDigPoint>& tDigitizer, Qt3DCore::QEntity* parent)
{
    //Create renderable 3D entity
    m_pParentEntity = parent;
    m_pRenderable3DEntity = new Renderable3DEntity(parent);

    QMatrix4x4 m;
    Qt3DCore::QTransform* transform =  new Qt3DCore::QTransform();
    m.rotate(180, QVector3D(0.0f, 1.0f, 0.0f));
    m.rotate(-90, QVector3D(1.0f, 0.0f, 0.0f));
    transform->setMatrix(m);
    m_pRenderable3DEntity->addComponent(transform);

    //Create sources as small 3D spheres
    QVector3D pos;
    Qt3DCore::QEntity* sourceSphereEntity;
    Qt3DExtras::QSphereMesh* sourceSphere;
    Qt3DExtras::QPhongMaterial* material;

    for(int i = 0; i < tDigitizer.size(); ++i) {

        pos.setX(tDigitizer[i].r[0]);
        pos.setY(tDigitizer[i].r[1]);
        pos.setZ(tDigitizer[i].r[2]);

        sourceSphereEntity = new Qt3DCore::QEntity();

        sourceSphere = new Qt3DExtras::QSphereMesh();
        if (tDigitizer[i].kind==FIFFV_POINT_CARDINAL){
            sourceSphere->setRadius(0.002f);
        }
        else{
            sourceSphere->setRadius(0.001f);
        }
        sourceSphereEntity->addComponent(sourceSphere);

        transform = new Qt3DCore::QTransform();
        QMatrix4x4 m;
        m.translate(pos);
        transform->setMatrix(m);
        sourceSphereEntity->addComponent(transform);

        material = new Qt3DExtras::QPhongMaterial();
        switch (tDigitizer[i].kind) {
        case FIFFV_POINT_CARDINAL:
            material->setAmbient(Qt::yellow);
            break;
        case FIFFV_POINT_HPI:
            material->setAmbient(Qt::red);
            break;
        case FIFFV_POINT_EEG:
            material->setAmbient(Qt::green);
            break;
        case FIFFV_POINT_EXTRA:
            material->setAmbient(Qt::blue);
            break;
        default:
            material->setAmbient(Qt::white);
            break;
        }
        sourceSphereEntity->addComponent(material);

        sourceSphereEntity->setParent(m_pRenderable3DEntity);
    }
    return true;
}


//*************************************************************************************************************

void DigitizerTreeItem::setVisible(bool state)
{
    m_pRenderable3DEntity->setParent(state ? m_pParentEntity : Q_NULLPTR);
}


//*************************************************************************************************************

void DigitizerTreeItem::onCheckStateChanged(const Qt::CheckState& checkState)
{
    this->setVisible(checkState==Qt::Unchecked ? false : true);
}
