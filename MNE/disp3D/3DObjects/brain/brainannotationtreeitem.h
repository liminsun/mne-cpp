//=============================================================================================================
/**
* @file     brainannotationtreeitem.h
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
* @brief     BrainAnnotationTreeItem class declaration.
*
*/

#ifndef BRAINANNOTATIONTREEITEM_H
#define BRAINANNOTATIONTREEITEM_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "../../disp3D_global.h"
#include "../../helpers/abstracttreeitem.h"
#include "../../helpers/types.h"


//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <Eigen/Core>


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

namespace FSLIB {
    class Label;
    class Surface;
    class Annotation;
}


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE DISP3DLIB
//=============================================================================================================

namespace DISP3DLIB
{


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================


//=============================================================================================================
/**
* BrainAnnotationTreeItem provides a generic item to hold information about brain annotation information.
*
* @brief Provides a generic brain tree item.
*/
class DISP3DNEWSHARED_EXPORT BrainAnnotationTreeItem : public AbstractTreeItem
{
    Q_OBJECT

public:
    typedef QSharedPointer<BrainAnnotationTreeItem> SPtr;             /**< Shared pointer type for BrainAnnotationTreeItem class. */
    typedef QSharedPointer<const BrainAnnotationTreeItem> ConstSPtr;  /**< Const shared pointer type for BrainAnnotationTreeItem class. */

    //=========================================================================================================
    /**
    * Default constructor.
    *
    * @param[in] iType      The type of the item. See types.h for declaration and definition.
    * @param[in] text       The text of this item. This is also by default the displayed name of the item in a view.
    */
    explicit BrainAnnotationTreeItem(int iType = Data3DTreeModelItemTypes::AnnotationItem, const QString& text = "Annotation" );

    //=========================================================================================================
    /**
    * Default destructor
    */
    ~BrainAnnotationTreeItem();

    //=========================================================================================================
    /**
    * AbstractTreeItem functions
    */
    QVariant data(int role = Qt::UserRole + 1) const;
    void setData(const QVariant& value, int role = Qt::UserRole + 1);

    //=========================================================================================================
    /**
    * Adds FreeSurfer data based on annotation information to this item.
    *
    * @param[in] tSurface           FreeSurfer surface.
    * @param[in] tAnnotation        FreeSurfer annotation.
    *
    * @return                       Returns true if successful.
    */
    bool addData(const FSLIB::Surface& tSurface, const FSLIB::Annotation& tAnnotation);

private:
    //=========================================================================================================
    /**
    * Call this function whenever the check box of this item was checked.
    *
    * @param[in] checkState        The current checkstate.
    */
    virtual void onCheckStateChanged(const Qt::CheckState& checkState);

signals:
    //=========================================================================================================
    /**
    * Emit this signal whenever the visibilty of the annotation set was changed.
    *
    * @param[in] isVisible      The visibility flag.
    */
    void annotationVisibiltyChanged(bool isVisible);
};

} //NAMESPACE DISP3DLIB

#endif // BRAINANNOTATIONTREEITEM_H
