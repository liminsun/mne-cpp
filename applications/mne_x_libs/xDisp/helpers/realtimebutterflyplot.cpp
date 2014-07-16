
//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "realtimebutterflyplot.h"


//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================

#include <QDebug>

//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace XDISPLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

RealTimeButterflyPlot::RealTimeButterflyPlot(QWidget *parent)
: QWidget(parent)
, m_pRealTimeEvokedModel(NULL)
{
}


//*************************************************************************************************************

void RealTimeButterflyPlot::dataUpdate(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    qDebug() << "RealTimeButterflyPlot::dataUpdate";
}
