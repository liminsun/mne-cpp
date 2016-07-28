//=============================================================================================================
/**
* @file     view3D.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     November, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    View3D class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "view3D.h"
#include "helpers/renderable3Dentity.h"
#include "helpers/types.h"

#include <mne/mne_sourceestimate.h>
#include <fiff/fiff_dig_point_set.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QDebug>
#include <QPropertyAnimation>
#include <QKeyEvent>

#include <Qt3DCore/QAspectEngine>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QForwardRenderer>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNELIB;
using namespace DISP3DLIB;
using namespace FSLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

View3D::View3D()
: Qt3DExtras::Qt3DWindow()
, m_pRootEntity(new Qt3DCore::QEntity())
, m_pCameraEntity(this->camera())
, m_pData3DTreeModel(Data3DTreeModel::SPtr(new Data3DTreeModel(0, m_pRootEntity)))
, m_bCameraRotationMode(false)
, m_bCameraTransMode(false)
, m_bModelRotationMode(false)
, m_vecCameraTrans(QVector3D(0.0,0.0,-0.5))
, m_vecCameraTransOld(QVector3D(0.0,0.0,-0.5))
, m_vecCameraRotation(QVector3D(0.0,0.0,0.0))
, m_vecCameraRotationOld(QVector3D(0.0,0.0,0.0))
, m_pCameraTransform(new Qt3DCore::QTransform())
{
    initMetatypes();
    init();
}


//*************************************************************************************************************

View3D::~View3D()
{
}


//*************************************************************************************************************

void View3D::initMetatypes()
{
    qRegisterMetaType<QByteArray>();
    qRegisterMetaType<QPair<QByteArray, QByteArray> >();

    qRegisterMetaType<Eigen::MatrixX3i>();
    qRegisterMetaType<Eigen::MatrixXd>();
    qRegisterMetaType<Eigen::MatrixX3f>();
    qRegisterMetaType<Eigen::VectorXf>();
    qRegisterMetaType<Eigen::VectorXi>();
    qRegisterMetaType<Eigen::VectorXd>();
    qRegisterMetaType<Eigen::RowVectorXf>();
    qRegisterMetaType<Eigen::Vector3f>();

    qRegisterMetaType<MatrixX3i>();
    qRegisterMetaType<MatrixXd>();
    qRegisterMetaType<MatrixX3f>();
    qRegisterMetaType<VectorXf>();
    qRegisterMetaType<VectorXi>();
    qRegisterMetaType<VectorXd>();
    qRegisterMetaType<RowVectorXf>();
    qRegisterMetaType<Vector3f>();
}


//*************************************************************************************************************

void View3D::init()
{
    //Light source
//    Qt3DRender::QPointLight *light1 = new Qt3DRender::QPointLight();
//    light1->setColor(Qt::white);
//    light1->setIntensity(0.1f);
//    m_pRootEntity->addComponent(light1);

    // Camera
    m_pCameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.0001f, 100000.0f);
    m_pCameraEntity->setPosition(m_vecCameraTrans);
    m_pCameraEntity->setUpVector(QVector3D(0, 1, 0));
    m_pCameraEntity->setViewCenter(QVector3D(0, 0, 0));

    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(m_pRootEntity);
    camController->setCamera(m_pCameraEntity);

    this->defaultFramegraph()->setClearColor(QColor::fromRgbF(0.0, 0.0, 0.0, 0.5));

    //Init the transforms
    initTransformations();

    // Set root object of the scene
    this->setRootEntity(m_pRootEntity);

    createCoordSystem(m_pRootEntity);
}


//*************************************************************************************************************

void View3D::initTransformations()
{
    // Initialize camera transforms
    m_pCameraTransform->setTranslation(m_vecCameraTrans);
    m_pCameraEntity->addComponent(m_pCameraTransform);
}


//*************************************************************************************************************

bool View3D::addBrainData(const QString& subject, const QString& set, const SurfaceSet& tSurfaceSet, const AnnotationSet& tAnnotationSet)
{
    return m_pData3DTreeModel->addData(subject, set, tSurfaceSet, tAnnotationSet);
}


//*************************************************************************************************************

bool View3D::addBrainData(const QString& subject, const QString& set, const Surface& tSurface, const Annotation& tAnnotation)
{
    return m_pData3DTreeModel->addData(subject, set, tSurface, tAnnotation);
}


//*************************************************************************************************************

bool View3D::addBrainData(const QString& subject, const QString& set, const MNESourceSpace& tSourceSpace)
{
    return m_pData3DTreeModel->addData(subject, set, tSourceSpace);
}


//*************************************************************************************************************

bool View3D::addBrainData(const QString& subject, const QString& set, const MNEForwardSolution& tForwardSolution)
{
    return m_pData3DTreeModel->addData(subject, set, tForwardSolution.src);
}


//*************************************************************************************************************

QList<BrainRTSourceLocDataTreeItem*> View3D::addRtBrainData(const QString& subject, const QString& set, const MNESourceEstimate& tSourceEstimate, const MNEForwardSolution& tForwardSolution)
{
    return m_pData3DTreeModel->addData(subject, set, tSourceEstimate, tForwardSolution);
}


//*************************************************************************************************************

bool View3D::addBemData(const QString& subject, const QString& set, const MNELIB::MNEBem& tBem)
{
    return m_pData3DTreeModel->addData(subject, set, tBem);
}


//*************************************************************************************************************

bool View3D::addDigitizerData(const QString& subject, const QString& set, const FiffDigPointSet& tDigitizer)
{
    return m_pData3DTreeModel->addData(subject, set, tDigitizer);
}


//*************************************************************************************************************

Data3DTreeModel* View3D::getData3DTreeModel()
{
    return m_pData3DTreeModel.data();
}


//*************************************************************************************************************

void View3D::setSceneColor(const QColor& colSceneColor)
{
    this->defaultFramegraph()->setClearColor(colSceneColor);
}


//*************************************************************************************************************

Qt3DCore::QEntity* View3D::get3DRootEntity()
{
    return m_pRootEntity;
}


//*************************************************************************************************************

void View3D::startModelRotation()
{
    //Start animation
    m_lPropertyAnimations.clear();

    for(int i = 0; i < m_pRootEntity->children().size(); ++i) {
        if(Renderable3DEntity* pItem = dynamic_cast<Renderable3DEntity*>(m_pRootEntity->children().at(i))) {
            QPropertyAnimation *anim = new QPropertyAnimation(pItem, QByteArrayLiteral("rotZ"));
            anim->setDuration(30000);
            anim->setStartValue(QVariant::fromValue(pItem->rotZ()));
            anim->setEndValue(QVariant::fromValue(pItem->rotZ() + 360.0f));
            anim->setLoopCount(-1);
            anim->start();
            m_lPropertyAnimations << anim;
        }
    }
}


//*************************************************************************************************************

void View3D::stopModelRotation()
{
    for(int i = 0; i < m_lPropertyAnimations.size(); ++i) {
        m_lPropertyAnimations.at(i)->stop();
    }
}


//*************************************************************************************************************

void View3D::keyPressEvent(QKeyEvent* e)
{
    switch ( e->key() )
    {
        case Qt::Key_Space:
            m_bModelRotationMode = true;
            break;

        default:
            Qt3DWindow::keyPressEvent(e);
    }
}


//*************************************************************************************************************

void View3D::keyReleaseEvent(QKeyEvent* e)
{
    switch ( e->key() )
    {
        case Qt::Key_Space:
            m_bModelRotationMode = false;
            break;

        default:
            Qt3DWindow::keyPressEvent(e);
    }
}


//*************************************************************************************************************

void View3D::mousePressEvent(QMouseEvent* e)
{
    m_mousePressPositon = e->pos();

    switch (e->button()) {
        case Qt::LeftButton:
            //TODO: SelectionMode
            break;
        case Qt::MidButton:
            m_bCameraRotationMode = true;
            break;
        case Qt::RightButton:
            m_bCameraTransMode = true;
            break;

        default:
            Qt3DWindow::mousePressEvent(e);
    }    
}


//*************************************************************************************************************

void View3D::wheelEvent(QWheelEvent* e)
{
    if(e->angleDelta().y() > 0)
        m_vecCameraTrans.setZ(m_vecCameraTrans.z() + 0.05f);
    else
        m_vecCameraTrans.setZ(m_vecCameraTrans.z() - 0.05f);

    // Transform
    m_pCameraTransform->setTranslation(m_vecCameraTrans);

    Qt3DWindow::wheelEvent(e);
}


//*************************************************************************************************************

void View3D::mouseReleaseEvent(QMouseEvent* e)
{
    m_bCameraRotationMode = false;
    m_bCameraTransMode = false;
    m_vecCameraTransOld = m_vecCameraTrans;
    m_vecCameraRotationOld = m_vecCameraRotation;

    Qt3DWindow::mouseReleaseEvent(e);
}


//*************************************************************************************************************

void View3D::mouseMoveEvent(QMouseEvent* e)
{
    if(m_bCameraRotationMode) {
        m_vecCameraRotation.setX(((e->pos().y() - m_mousePressPositon.y()) * 0.1f) + m_vecCameraRotationOld.x());
        m_vecCameraRotation.setY(((e->pos().x() - m_mousePressPositon.x()) * 0.1f) + m_vecCameraRotationOld.y());

        //Rotate all surface objects
        if(!m_bModelRotationMode) {
            //Rotate camera
            m_pCameraTransform->setRotationX(m_vecCameraRotation.x());
            m_pCameraTransform->setRotationY(m_vecCameraRotation.y());
        } else {
            //Rotate all surface objects
            for(int i = 0; i < m_pRootEntity->children().size(); ++i) {
                if(Renderable3DEntity* pItem = dynamic_cast<Renderable3DEntity*>(m_pRootEntity->children().at(i))) {
                    pItem->setRotZ(m_vecCameraRotation.y());
                    pItem->setRotX(90+m_vecCameraRotation.x());
                }
            }
        }
    }

    if(m_bCameraTransMode) {
        m_vecCameraTrans.setX(((e->pos().x() - m_mousePressPositon.x()) * 0.0001f) + m_vecCameraTransOld.x());
        m_vecCameraTrans.setY(((e->pos().y() - m_mousePressPositon.y()) * -0.0001f) + m_vecCameraTransOld.y());

        // Camera translation transform
        m_pCameraTransform->setTranslation(m_vecCameraTrans);
    }

    Qt3DWindow::mouseMoveEvent(e);
}


//*************************************************************************************************************

void View3D::createCoordSystem(Qt3DCore::QEntity* parent)
{
    // Y - red
    Qt3DExtras::QCylinderMesh *YAxis = new Qt3DExtras::QCylinderMesh();
    YAxis->setRadius(0.001f);
    YAxis->setLength(30);
    YAxis->setRings(100);
    YAxis->setSlices(20);

    m_YAxisEntity = QSharedPointer<Qt3DCore::QEntity>(new Qt3DCore::QEntity(parent));
    m_YAxisEntity->addComponent(YAxis);

    Qt3DExtras::QPhongMaterial *phongMaterialY = new Qt3DExtras::QPhongMaterial();
    phongMaterialY->setDiffuse(QColor(255, 0, 0));
    phongMaterialY->setAmbient(Qt::gray);
    phongMaterialY->setSpecular(Qt::white);
    phongMaterialY->setShininess(50.0f);
    m_YAxisEntity->addComponent(phongMaterialY);

    // Z - blue
    Qt3DExtras::QCylinderMesh *ZAxis = new Qt3DExtras::QCylinderMesh();
    ZAxis->setRadius(0.001f);
    ZAxis->setLength(30);
    ZAxis->setRings(100);
    ZAxis->setSlices(20);

    Qt3DCore::QTransform *transformZ = new Qt3DCore::QTransform();
    transformZ->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 90));

    m_ZAxisEntity = QSharedPointer<Qt3DCore::QEntity>(new Qt3DCore::QEntity(parent));
    m_ZAxisEntity->addComponent(ZAxis);
    m_ZAxisEntity->addComponent(transformZ);

    Qt3DExtras::QPhongMaterial *phongMaterialZ = new Qt3DExtras::QPhongMaterial();
    phongMaterialZ->setDiffuse(QColor(0, 0, 255));
    phongMaterialZ->setAmbient(Qt::gray);
    phongMaterialZ->setSpecular(Qt::white);
    phongMaterialZ->setShininess(50.0f);
    m_ZAxisEntity->addComponent(phongMaterialZ);

    // X - green
    Qt3DExtras::QCylinderMesh *XAxis = new Qt3DExtras::QCylinderMesh();
    XAxis->setRadius(0.001f);
    XAxis->setLength(30);
    XAxis->setRings(100);
    XAxis->setSlices(20);

    Qt3DCore::QTransform *transformX = new Qt3DCore::QTransform();
    transformX->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), 90));

    m_XAxisEntity = QSharedPointer<Qt3DCore::QEntity>(new Qt3DCore::QEntity(parent));
    m_XAxisEntity->addComponent(XAxis);
    m_XAxisEntity->addComponent(transformX);

    Qt3DExtras::QPhongMaterial *phongMaterialX = new Qt3DExtras::QPhongMaterial();
    phongMaterialX->setDiffuse(QColor(0, 255, 0));
    phongMaterialX->setAmbient(Qt::gray);
    phongMaterialX->setSpecular(Qt::white);
    phongMaterialX->setShininess(50.0f);
    m_XAxisEntity->addComponent(phongMaterialX);
}
