//=============================================================================================================
/**
* @file     plot.h
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     June, 2013
*
* @section  LICENSE
*
* Copyright (C) 2013, Christoph Dinh and Matti Hamalainen. All rights reserved.
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
* @brief    Plot class declaration
*
*/
#ifndef PLOT_H
#define PLOT_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "disp_global.h"
#include "graph.h"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QWidget>
#include <QString>
#include <QList>
#include <QVector>
#include <QPointF>
#include <QSharedPointer>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <Eigen/Core>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE DISP3DLIB
//=============================================================================================================

namespace DISPLIB
{

//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace Eigen;


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================



//=============================================================================================================
/**
* Plots vector data, similiar to MATLABs plot
*
* @brief Vector plot
*/
class DISPSHARED_EXPORT Plot : public Graph
{
    Q_OBJECT
public:
    typedef QSharedPointer<Plot> SPtr;            /**< Shared pointer type for MatrixView class. */
    typedef QSharedPointer<const Plot> ConstSPtr; /**< Const shared pointer type for MatrixView class. */

    //=========================================================================================================
    /**
    * Creates the plot.
    *
    * @param[in] parent     Parent QObject (optional)
    */
    explicit Plot(QWidget *parent = 0);

    //=========================================================================================================
    /**
    * Creates the plot using a given double vector.
    *
    * @param[in] p_dVec     The double data vector
    * @param[in] parent     Parent QObject (optional)
    */
    explicit Plot(VectorXd &p_dVec, QWidget *parent = 0);

    //=========================================================================================================
    /**
    * Destructs the Plot object
    */
    ~Plot();

    //=========================================================================================================
    /**
    * Initializes the Plot object
    */
    void init();

    //=========================================================================================================
    /**
    * Updates the plot using a given double vector without given X data.
    *
    * @param[in] p_dVec     The double data vector
    */
    void updateData(VectorXd &p_dVec);

protected:
    void paintEvent(QPaintEvent*);

    bool m_bHoldOn;             /**< If multiple plots */

    QList<QVector<QPointF> > m_qListVecPointFPaths;

    double m_dMinX;             /**< Minimal X value */
    double m_dMaxX;             /**< Maximal X value */
    double m_dMinY;             /**< Minimal Y value */
    double m_dMaxY;             /**< Maximal Y value */
};

//*************************************************************************************************************
//=============================================================================================================
// INLINE DEFINITIONS
//=============================================================================================================

} // NAMESPACE

#endif // PLOT_H
