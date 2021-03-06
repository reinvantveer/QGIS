/***************************************************************************
                         qgslayoutlegendwidget.cpp
                         -------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutlegendwidget.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutlegendlayersdialog.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgsguiutils.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerlegend.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgslayoutitemlegend.h"

#include <QMessageBox>
#include <QInputDialog>


static int _unfilteredLegendNodeIndex( QgsLayerTreeModelLegendNode *legendNode )
{
  return legendNode->model()->layerOriginalLegendNodes( legendNode->layerNode() ).indexOf( legendNode );
}

static int _originalLegendNodeIndex( QgsLayerTreeModelLegendNode *legendNode )
{
  // figure out index of the legend node as it comes out of the map layer legend.
  // first legend nodes may be reordered, output of that is available in layerOriginalLegendNodes().
  // next the nodes may be further filtered (by scale, map content etc).
  // so here we go in reverse order: 1. find index before filtering, 2. find index before reorder
  int unfilteredNodeIndex = _unfilteredLegendNodeIndex( legendNode );
  QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( legendNode->layerNode() );
  return ( unfilteredNodeIndex >= 0 && unfilteredNodeIndex < order.count() ? order[unfilteredNodeIndex] : -1 );
}


QgsLayoutLegendWidget::QgsLayoutLegendWidget( QgsLayoutItemLegend *legend )
  : QgsLayoutItemBaseWidget( nullptr, legend )
  , mLegend( legend )
{
  Q_ASSERT( mLegend );

  setupUi( this );
  connect( mWrapCharLineEdit, &QLineEdit::textChanged, this, &QgsLayoutLegendWidget::mWrapCharLineEdit_textChanged );
  connect( mTitleLineEdit, &QLineEdit::textChanged, this, &QgsLayoutLegendWidget::mTitleLineEdit_textChanged );
  connect( mTitleAlignCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutLegendWidget::mTitleAlignCombo_currentIndexChanged );
  connect( mColumnCountSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mColumnCountSpinBox_valueChanged );
  connect( mSplitLayerCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mSplitLayerCheckBox_toggled );
  connect( mEqualColumnWidthCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mEqualColumnWidthCheckBox_toggled );
  connect( mSymbolWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSymbolWidthSpinBox_valueChanged );
  connect( mSymbolHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSymbolHeightSpinBox_valueChanged );
  connect( mWmsLegendWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mWmsLegendWidthSpinBox_valueChanged );
  connect( mWmsLegendHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mWmsLegendHeightSpinBox_valueChanged );
  connect( mTitleSpaceBottomSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mTitleSpaceBottomSpinBox_valueChanged );
  connect( mGroupSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mGroupSpaceSpinBox_valueChanged );
  connect( mLayerSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mLayerSpaceSpinBox_valueChanged );
  connect( mSymbolSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSymbolSpaceSpinBox_valueChanged );
  connect( mIconLabelSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mIconLabelSpaceSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutLegendWidget::mFontColorButton_colorChanged );
  connect( mBoxSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mBoxSpaceSpinBox_valueChanged );
  connect( mColumnSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mColumnSpaceSpinBox_valueChanged );
  connect( mLineSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mLineSpacingSpinBox_valueChanged );
  connect( mCheckBoxAutoUpdate, &QCheckBox::stateChanged, this, &QgsLayoutLegendWidget::mCheckBoxAutoUpdate_stateChanged );
  connect( mCheckboxResizeContents, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mCheckboxResizeContents_toggled );
  connect( mRasterStrokeGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutLegendWidget::mRasterStrokeGroupBox_toggled );
  connect( mRasterStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mRasterStrokeWidthSpinBox_valueChanged );
  connect( mRasterStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutLegendWidget::mRasterStrokeColorButton_colorChanged );
  connect( mMoveDownToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mMoveDownToolButton_clicked );
  connect( mMoveUpToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mMoveUpToolButton_clicked );
  connect( mRemoveToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mRemoveToolButton_clicked );
  connect( mAddToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mAddToolButton_clicked );
  connect( mEditPushButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mEditPushButton_clicked );
  connect( mCountToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mCountToolButton_clicked );
  connect( mExpressionFilterButton, &QgsLegendFilterButton::toggled, this, &QgsLayoutLegendWidget::mExpressionFilterButton_toggled );
  connect( mFilterByMapToolButton, &QToolButton::toggled, this, &QgsLayoutLegendWidget::mFilterByMapToolButton_toggled );
  connect( mUpdateAllPushButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mUpdateAllPushButton_clicked );
  connect( mAddGroupToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mAddGroupToolButton_clicked );
  connect( mFilterLegendByAtlasCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mFilterLegendByAtlasCheckBox_toggled );
  connect( mItemTreeView, &QgsLayerTreeView::doubleClicked, this, &QgsLayoutLegendWidget::mItemTreeView_doubleClicked );
  setPanelTitle( tr( "Legend properties" ) );

  mTitleFontButton->setMode( QgsFontButton::ModeQFont );
  mGroupFontButton->setMode( QgsFontButton::ModeQFont );
  mLayerFontButton->setMode( QgsFontButton::ModeQFont );
  mItemFontButton->setMode( QgsFontButton::ModeQFont );

  // setup icons
  mAddToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mEditPushButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  mRemoveToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  mMoveUpToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowUp.svg" ) ) );
  mMoveDownToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowDown.svg" ) ) );
  mCountToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionSum.svg" ) ) );

  mFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mFontColorButton->setContext( QStringLiteral( "composer" ) );

  mRasterStrokeColorButton->setColorDialogTitle( tr( "Select Stroke Color" ) );
  mRasterStrokeColorButton->setAllowOpacity( true );
  mRasterStrokeColorButton->setContext( QStringLiteral( "composer " ) );

  mMapComboBox->setCurrentLayout( legend->layout() );
  mMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
  connect( mMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutLegendWidget::composerMapChanged );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, legend );
  mainLayout->addWidget( mItemPropertiesWidget );

  mItemTreeView->setHeaderHidden( true );

  mItemTreeView->setModel( legend->model() );
  mItemTreeView->setMenuProvider( new QgsLayoutLegendMenuProvider( mItemTreeView, this ) );
  connect( legend, &QgsLayoutObject::changed, this, &QgsLayoutLegendWidget::setGuiElements );

#if 0 //TODO
  // connect atlas state to the filter legend by atlas checkbox
  connect( &legend->composition()->atlasComposition(), &QgsAtlasComposition::toggled, this, &QgsLayoutLegendWidget::updateFilterLegendByAtlasButton );
  connect( &legend->composition()->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, &QgsLayoutLegendWidget::updateFilterLegendByAtlasButton );
#endif

  registerDataDefinedButton( mLegendTitleDDBtn, QgsLayoutObject::LegendTitle );
  registerDataDefinedButton( mColumnsDDBtn, QgsLayoutObject::LegendColumnCount );

  setGuiElements();

  connect( mItemTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsLayoutLegendWidget::selectedChanged );
  connect( mTitleFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::titleFontChanged );
  connect( mGroupFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::groupFontChanged );
  connect( mLayerFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::layerFontChanged );
  connect( mItemFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::itemFontChanged );
}

void QgsLayoutLegendWidget::setGuiElements()
{
  if ( !mLegend )
  {
    return;
  }

  int alignment = mLegend->titleAlignment() == Qt::AlignLeft ? 0 : mLegend->titleAlignment() == Qt::AlignHCenter ? 1 : 2;

  blockAllSignals( true );
  mTitleLineEdit->setText( mLegend->title() );
  mTitleAlignCombo->setCurrentIndex( alignment );
  mFilterByMapToolButton->setChecked( mLegend->legendFilterByMapEnabled() );
  mColumnCountSpinBox->setValue( mLegend->columnCount() );
  mSplitLayerCheckBox->setChecked( mLegend->splitLayer() );
  mEqualColumnWidthCheckBox->setChecked( mLegend->equalColumnWidth() );
  mSymbolWidthSpinBox->setValue( mLegend->symbolWidth() );
  mSymbolHeightSpinBox->setValue( mLegend->symbolHeight() );
  mWmsLegendWidthSpinBox->setValue( mLegend->wmsLegendWidth() );
  mWmsLegendHeightSpinBox->setValue( mLegend->wmsLegendHeight() );
  mTitleSpaceBottomSpinBox->setValue( mLegend->style( QgsLegendStyle::Title ).margin( QgsLegendStyle::Bottom ) );
  mGroupSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Group ).margin( QgsLegendStyle::Top ) );
  mLayerSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Subgroup ).margin( QgsLegendStyle::Top ) );
  // We keep Symbol and SymbolLabel Top in sync for now
  mSymbolSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top ) );
  mIconLabelSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left ) );
  mBoxSpaceSpinBox->setValue( mLegend->boxSpace() );
  mColumnSpaceSpinBox->setValue( mLegend->columnSpace() );
  mLineSpacingSpinBox->setValue( mLegend->lineSpacing() );

  mRasterStrokeGroupBox->setChecked( mLegend->drawRasterStroke() );
  mRasterStrokeWidthSpinBox->setValue( mLegend->rasterStrokeWidth() );
  mRasterStrokeColorButton->setColor( mLegend->rasterStrokeColor() );

  mCheckBoxAutoUpdate->setChecked( mLegend->autoUpdateModel() );

  mCheckboxResizeContents->setChecked( mLegend->resizeToContents() );
  mFilterLegendByAtlasCheckBox->setChecked( mLegend->legendFilterOutAtlas() );
  mWrapCharLineEdit->setText( mLegend->wrapString() );

  QgsLayoutItemMap *map = mLegend->map();
  mMapComboBox->setItem( map );
  mFontColorButton->setColor( mLegend->fontColor() );
  mTitleFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Title ).font() );
  mGroupFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Group ).font() );
  mLayerFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Subgroup ).font() );
  mItemFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::SymbolLabel ).font() );

  blockAllSignals( false );

  mCheckBoxAutoUpdate_stateChanged( mLegend->autoUpdateModel() ? Qt::Checked : Qt::Unchecked );
  updateDataDefinedButton( mLegendTitleDDBtn );
  updateDataDefinedButton( mColumnsDDBtn );
}

void QgsLayoutLegendWidget::mWrapCharLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Legend Wrap" ) );
    mLegend->setWrapString( text );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mTitleLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Legend Title" ), QgsLayoutItem::UndoLegendText );
    mLegend->setTitle( text );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mTitleAlignCombo_currentIndexChanged( int index )
{
  if ( mLegend )
  {
    Qt::AlignmentFlag alignment = index == 0 ? Qt::AlignLeft : index == 1 ? Qt::AlignHCenter : Qt::AlignRight;
    mLegend->beginCommand( tr( "Change Title Alignment" ) );
    mLegend->setTitleAlignment( alignment );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mColumnCountSpinBox_valueChanged( int c )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Column Count" ), QgsLayoutItem::UndoLegendColumnCount );
    mLegend->setColumnCount( c );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
  mSplitLayerCheckBox->setEnabled( c > 1 );
  mEqualColumnWidthCheckBox->setEnabled( c > 1 );
}

void QgsLayoutLegendWidget::mSplitLayerCheckBox_toggled( bool checked )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Split Legend Layers" ) );
    mLegend->setSplitLayer( checked );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mEqualColumnWidthCheckBox_toggled( bool checked )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend Column Width" ) );
    mLegend->setEqualColumnWidth( checked );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSymbolWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize Symbol Width" ), QgsLayoutItem::UndoLegendSymbolWidth );
    mLegend->setSymbolWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSymbolHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize Symbol Height" ), QgsLayoutItem::UndoLegendSymbolHeight );
    mLegend->setSymbolHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mWmsLegendWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize WMS Width" ), QgsLayoutItem::UndoLegendWmsLegendWidth );
    mLegend->setWmsLegendWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mWmsLegendHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize WMS Height" ), QgsLayoutItem::UndoLegendWmsLegendHeight );
    mLegend->setWmsLegendHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mTitleSpaceBottomSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Title Space" ), QgsLayoutItem::UndoLegendTitleSpaceBottom );
    mLegend->rstyle( QgsLegendStyle::Title ).setMargin( QgsLegendStyle::Bottom, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mGroupSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Space" ), QgsLayoutItem::UndoLegendGroupSpace );
    mLegend->rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mLayerSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Layer Space" ), QgsLayoutItem::UndoLegendLayerSpace );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSymbolSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Symbol Space" ), QgsLayoutItem::UndoLegendSymbolSpace );
    // We keep Symbol and SymbolLabel Top in sync for now
    mLegend->rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, d );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mIconLabelSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Label Space" ), QgsLayoutItem::UndoLegendIconSymbolSpace );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::titleFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Title Font" ), QgsLayoutItem::UndoLegendTitleFont );
    mLegend->setStyleFont( QgsLegendStyle::Title, mTitleFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::groupFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Font" ), QgsLayoutItem::UndoLegendGroupFont );
    mLegend->setStyleFont( QgsLegendStyle::Group, mGroupFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::layerFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Layer Font" ), QgsLayoutItem::UndoLegendLayerFont );
    mLegend->setStyleFont( QgsLegendStyle::Subgroup, mLayerFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::itemFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Item Font" ), QgsLayoutItem::UndoLegendItemFont );
    mLegend->setStyleFont( QgsLegendStyle::SymbolLabel, mItemFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mFontColorButton_colorChanged( const QColor &newFontColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Change Font Color" ), QgsLayoutItem::UndoLegendFontColor );
  mLegend->setFontColor( newFontColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mBoxSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Box Space" ), QgsLayoutItem::UndoLegendBoxSpace );
    mLegend->setBoxSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mColumnSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Column Space" ), QgsLayoutItem::UndoLegendColumnSpace );
    mLegend->setColumnSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mLineSpacingSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Line Space" ), QgsLayoutItem::UndoLegendLineSpacing );
    mLegend->setLineSpacing( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

static void _moveLegendNode( QgsLayerTreeLayer *nodeLayer, int legendNodeIndex, int offset )
{
  QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

  if ( legendNodeIndex < 0 || legendNodeIndex >= order.count() )
    return;
  if ( legendNodeIndex + offset < 0 || legendNodeIndex + offset >= order.count() )
    return;

  int id = order.takeAt( legendNodeIndex );
  order.insert( legendNodeIndex + offset, id );

  QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
}


void QgsLayoutLegendWidget::mMoveDownToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  QModelIndex parentIndex = index.parent();
  if ( !index.isValid() || index.row() == mItemTreeView->model()->rowCount( parentIndex ) - 1 )
    return;

  QgsLayerTreeNode *node = mItemTreeView->layerTreeModel()->index2node( index );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( QStringLiteral( "Moved Legend Item Down" ) );

  if ( node )
  {
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( index.row() + 2, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), _unfilteredLegendNodeIndex( legendNode ), 1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->layerTreeModel()->index( index.row() + 1, 0, parentIndex ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mMoveUpToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  QModelIndex parentIndex = index.parent();
  if ( !index.isValid() || index.row() == 0 )
    return;

  QgsLayerTreeNode *node = mItemTreeView->layerTreeModel()->index2node( index );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( QStringLiteral( "Move Legend Item Up" ) );

  if ( node )
  {
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( index.row() - 1, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), _unfilteredLegendNodeIndex( legendNode ), -1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->layerTreeModel()->index( index.row() - 1, 0, parentIndex ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mCheckBoxAutoUpdate_stateChanged( int state )
{
  mLegend->beginCommand( QStringLiteral( "Change Auto Update" ) );

  mLegend->setAutoUpdateModel( state == Qt::Checked );

  mLegend->updateFilterByMap();
  mLegend->endCommand();

  // do not allow editing of model if auto update is on - we would modify project's layer tree
  QList<QWidget *> widgets;
  widgets << mMoveDownToolButton << mMoveUpToolButton << mRemoveToolButton << mAddToolButton
          << mEditPushButton << mCountToolButton << mUpdateAllPushButton << mAddGroupToolButton
          << mExpressionFilterButton;
  for ( QWidget *w : qgis::as_const( widgets ) )
    w->setEnabled( state != Qt::Checked );

  if ( state == Qt::Unchecked )
  {
    // update widgets state based on current selection
    selectedChanged( QModelIndex(), QModelIndex() );
  }
}

void QgsLayoutLegendWidget::composerMapChanged( QgsLayoutItem *item )
{
  if ( !mLegend )
  {
    return;
  }

  QgsLayout *layout = mLegend->layout();
  if ( !layout )
  {
    return;
  }

  QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( item );
  if ( map )
  {
    mLegend->beginCommand( tr( "Change Legend Map" ) );
    mLegend->setMap( map );
    mLegend->updateFilterByMap();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mCheckboxResizeContents_toggled( bool checked )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Resize Legend to Contents" ) );
  mLegend->setResizeToContents( checked );
  if ( checked )
    mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mRasterStrokeGroupBox_toggled( bool state )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Change Legend Borders" ) );
  mLegend->setDrawRasterStroke( state );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mRasterStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Resize Legend Borders" ), QgsLayoutItem::UndoLegendRasterStrokeWidth );
  mLegend->setRasterStrokeWidth( d );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mRasterStrokeColorButton_colorChanged( const QColor &newColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Change Legend Border Color" ), QgsLayoutItem::UndoLegendRasterStrokeColor );
  mLegend->setRasterStrokeColor( newColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mAddToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QgsLayoutLegendLayersDialog addDialog( this );
  if ( addDialog.exec() == QDialog::Accepted )
  {
    const QList<QgsMapLayer *> layers = addDialog.selectedLayers();
    if ( !layers.empty() )
    {
      mLegend->beginCommand( QStringLiteral( "Add Legend Item(s)" ) );
      for ( QgsMapLayer *layer : layers )
      {
        mLegend->model()->rootGroup()->addLayer( layer );
      }
      mLegend->updateLegend();
      mLegend->endCommand();
    }
  }
}

void QgsLayoutLegendWidget::mRemoveToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QItemSelectionModel *selectionModel = mItemTreeView->selectionModel();
  if ( !selectionModel )
  {
    return;
  }

  mLegend->beginCommand( QStringLiteral( "Remove Legend Item" ) );

  QList<QPersistentModelIndex> indexes;
  Q_FOREACH ( const QModelIndex &index, selectionModel->selectedIndexes() )
    indexes << index;

  // first try to remove legend nodes
  QHash<QgsLayerTreeLayer *, QList<int> > nodesWithRemoval;
  Q_FOREACH ( const QPersistentModelIndex &index, indexes )
  {
    if ( QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index ) )
    {
      QgsLayerTreeLayer *nodeLayer = legendNode->layerNode();
      nodesWithRemoval[nodeLayer].append( _unfilteredLegendNodeIndex( legendNode ) );
    }
  }
  Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, nodesWithRemoval.keys() )
  {
    QList<int> toDelete = nodesWithRemoval[nodeLayer];
    std::sort( toDelete.begin(), toDelete.end(), std::greater<int>() );
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

    Q_FOREACH ( int i, toDelete )
    {
      if ( i >= 0 && i < order.count() )
        order.removeAt( i );
    }

    QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );
  }

  // then remove layer tree nodes
  Q_FOREACH ( const QPersistentModelIndex &index, indexes )
  {
    if ( index.isValid() && mItemTreeView->layerTreeModel()->index2node( index ) )
      mLegend->model()->removeRow( index.row(), index.parent() );
  }

  mLegend->updateLegend();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mEditPushButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex idx = mItemTreeView->selectionModel()->currentIndex();
  mItemTreeView_doubleClicked( idx );
}

void QgsLayoutLegendWidget::resetLayerNodeToDefaults()
{
  if ( !mLegend )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsLayerTreeLayer *nodeLayer = nullptr;
  if ( QgsLayerTreeNode *node = mItemTreeView->layerTreeModel()->index2node( currentIndex ) )
  {
    if ( QgsLayerTree::isLayer( node ) )
      nodeLayer = QgsLayerTree::toLayer( node );
  }
  if ( QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( currentIndex ) )
  {
    nodeLayer = legendNode->layerNode();
  }

  if ( !nodeLayer )
    return;

  mLegend->beginCommand( tr( "Update Legend" ) );

  Q_FOREACH ( const QString &key, nodeLayer->customProperties() )
  {
    if ( key.startsWith( QLatin1String( "legend/" ) ) )
      nodeLayer->removeCustomProperty( key );
  }

  mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );

  mLegend->updateLegend();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mCountToolButton_clicked( bool checked )
{
  QgsDebugMsg( "Entered." );
  if ( !mLegend )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  mLegend->beginCommand( tr( "Update Legend" ) );
  currentNode->setCustomProperty( QStringLiteral( "showFeatureCount" ), checked ? 1 : 0 );
  mLegend->updateFilterByMap();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mFilterByMapToolButton_toggled( bool checked )
{
  mLegend->beginCommand( tr( "Update Legend" ) );
  mLegend->setLegendFilterByMapEnabled( checked );
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mExpressionFilterButton_toggled( bool checked )
{
  if ( !mLegend )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  QgsLayerTreeUtils::setLegendFilterByExpression( *qobject_cast<QgsLayerTreeLayer *>( currentNode ),
      mExpressionFilterButton->expressionText(),
      checked );

  mLegend->beginCommand( tr( "Update Legend" ) );
  mLegend->updateFilterByMap();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mUpdateAllPushButton_clicked()
{
  updateLegend();
}

void QgsLayoutLegendWidget::mAddGroupToolButton_clicked()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Add Legend Group" ) );
    mLegend->model()->rootGroup()->addGroup( tr( "Group" ) );
    mLegend->updateLegend();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mFilterLegendByAtlasCheckBox_toggled( bool toggled )
{
  Q_UNUSED( toggled );
  if ( mLegend )
  {
#if 0 //TODO
    mLegend->setLegendFilterOutAtlas( toggled );
    // force update of legend when in preview mode
    if ( mLegend->composition()->atlasMode() == QgsComposition::PreviewAtlas )
    {
      mLegend->composition()->atlasComposition().refreshFeature();
    }
#endif
  }
}

void QgsLayoutLegendWidget::updateLegend()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Update Legend" ) );

    // this will reset the model completely, losing any changes
    mLegend->setAutoUpdateModel( true );
    mLegend->setAutoUpdateModel( false );
    mLegend->updateFilterByMap();
    mLegend->endCommand();
  }
}

bool QgsLayoutLegendWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutLegend )
    return false;

  if ( mLegend )
  {
    disconnect( mLegend, &QgsLayoutObject::changed, this, &QgsLayoutLegendWidget::setGuiElements );
  }

  mLegend = qobject_cast< QgsLayoutItemLegend * >( item );
  mItemPropertiesWidget->setItem( mLegend );

  if ( mLegend )
  {
    mItemTreeView->setModel( mLegend->model() );
    connect( mLegend, &QgsLayoutObject::changed, this, &QgsLayoutLegendWidget::setGuiElements );
  }

  setGuiElements();

  return true;
}

void QgsLayoutLegendWidget::blockAllSignals( bool b )
{
  mTitleLineEdit->blockSignals( b );
  mTitleAlignCombo->blockSignals( b );
  mItemTreeView->blockSignals( b );
  mCheckBoxAutoUpdate->blockSignals( b );
  mMapComboBox->blockSignals( b );
  mFilterByMapToolButton->blockSignals( b );
  mColumnCountSpinBox->blockSignals( b );
  mSplitLayerCheckBox->blockSignals( b );
  mEqualColumnWidthCheckBox->blockSignals( b );
  mSymbolWidthSpinBox->blockSignals( b );
  mSymbolHeightSpinBox->blockSignals( b );
  mGroupSpaceSpinBox->blockSignals( b );
  mLayerSpaceSpinBox->blockSignals( b );
  mSymbolSpaceSpinBox->blockSignals( b );
  mIconLabelSpaceSpinBox->blockSignals( b );
  mBoxSpaceSpinBox->blockSignals( b );
  mColumnSpaceSpinBox->blockSignals( b );
  mFontColorButton->blockSignals( b );
  mRasterStrokeGroupBox->blockSignals( b );
  mRasterStrokeColorButton->blockSignals( b );
  mRasterStrokeWidthSpinBox->blockSignals( b );
  mWmsLegendWidthSpinBox->blockSignals( b );
  mWmsLegendHeightSpinBox->blockSignals( b );
  mCheckboxResizeContents->blockSignals( b );
  mTitleSpaceBottomSpinBox->blockSignals( b );
  mFilterLegendByAtlasCheckBox->blockSignals( b );
  mTitleFontButton->blockSignals( b );
  mGroupFontButton->blockSignals( b );
  mLayerFontButton->blockSignals( b );
  mItemFontButton->blockSignals( b );
  mWrapCharLineEdit->blockSignals( b );
}

void QgsLayoutLegendWidget::selectedChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( current );
  Q_UNUSED( previous );

  if ( mLegend && mLegend->autoUpdateModel() )
    return;

  mCountToolButton->setChecked( false );
  mCountToolButton->setEnabled( false );

  mExpressionFilterButton->blockSignals( true );
  mExpressionFilterButton->setChecked( false );
  mExpressionFilterButton->setEnabled( false );
  mExpressionFilterButton->blockSignals( false );

  QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  QgsLayerTreeLayer *currentLayerNode = QgsLayerTree::toLayer( currentNode );
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( currentLayerNode->layer() );
  if ( !vl )
    return;

  mCountToolButton->setChecked( currentNode->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt() );
  mCountToolButton->setEnabled( true );

  bool exprEnabled;
  QString expr = QgsLayerTreeUtils::legendFilterByExpression( *qobject_cast<QgsLayerTreeLayer *>( currentNode ), &exprEnabled );
  mExpressionFilterButton->blockSignals( true );
  mExpressionFilterButton->setExpressionText( expr );
  mExpressionFilterButton->setVectorLayer( vl );
  mExpressionFilterButton->setEnabled( true );
  mExpressionFilterButton->setChecked( exprEnabled );
  mExpressionFilterButton->blockSignals( false );
}

void QgsLayoutLegendWidget::setCurrentNodeStyleFromAction()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a || !mItemTreeView->currentNode() )
    return;

  QgsLegendRenderer::setNodeLegendStyle( mItemTreeView->currentNode(), ( QgsLegendStyle::Style ) a->data().toInt() );
  mLegend->updateFilterByMap();
}

void QgsLayoutLegendWidget::updateFilterLegendByAtlasButton()
{
#if 0 //TODO
  const QgsAtlasComposition &atlas = mLegend->composition()->atlasComposition();
  mFilterLegendByAtlasCheckBox->setEnabled( atlas.enabled() && atlas.coverageLayer() && atlas.coverageLayer()->geometryType() == QgsWkbTypes::PolygonGeometry );
#endif
}

void QgsLayoutLegendWidget::mItemTreeView_doubleClicked( const QModelIndex &idx )
{
  if ( !mLegend || !idx.isValid() )
  {
    return;
  }

  QgsLayerTreeModel *model = mItemTreeView->layerTreeModel();
  QgsLayerTreeNode *currentNode = model->index2node( idx );
  QgsLayerTreeModelLegendNode *legendNode = model->index2legendNode( idx );
  QString currentText;

  if ( QgsLayerTree::isGroup( currentNode ) )
  {
    currentText = QgsLayerTree::toGroup( currentNode )->name();
  }
  else if ( QgsLayerTree::isLayer( currentNode ) )
  {
    currentText = QgsLayerTree::toLayer( currentNode )->name();
    QVariant v = currentNode->customProperty( QStringLiteral( "legend/title-label" ) );
    if ( !v.isNull() )
      currentText = v.toString();
  }
  else if ( legendNode )
  {
    currentText = legendNode->data( Qt::EditRole ).toString();
  }

  bool ok;
  QString newText = QInputDialog::getText( this, tr( "Legend item properties" ), tr( "Item text" ),
                    QLineEdit::Normal, currentText, &ok );
  if ( !ok || newText == currentText )
    return;

  mLegend->beginCommand( tr( "Edit Legend Item" ) );

  if ( QgsLayerTree::isGroup( currentNode ) )
  {
    QgsLayerTree::toGroup( currentNode )->setName( newText );
  }
  else if ( QgsLayerTree::isLayer( currentNode ) )
  {
    currentNode->setCustomProperty( QStringLiteral( "legend/title-label" ), newText );

    // force update of label of the legend node with embedded icon (a bit clumsy i know)
    if ( QgsLayerTreeModelLegendNode *embeddedNode = model->legendNodeEmbeddedInParent( QgsLayerTree::toLayer( currentNode ) ) )
      embeddedNode->setUserLabel( QString() );
  }
  else if ( legendNode )
  {
    int originalIndex = _originalLegendNodeIndex( legendNode );
    QgsMapLayerLegendUtils::setLegendNodeUserLabel( legendNode->layerNode(), originalIndex, newText );
    model->refreshLayerLegend( legendNode->layerNode() );
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}


//
// QgsComposerLegendMenuProvider
//

QgsLayoutLegendMenuProvider::QgsLayoutLegendMenuProvider( QgsLayerTreeView *view, QgsLayoutLegendWidget *w )
  : mView( view )
  , mWidget( w )
{}

QMenu *QgsLayoutLegendMenuProvider::createContextMenu()
{
  if ( !mView->currentNode() )
    return nullptr;

  if ( mWidget->legend()->autoUpdateModel() )
    return nullptr; // no editing allowed

  QMenu *menu = new QMenu();

  if ( QgsLayerTree::isLayer( mView->currentNode() ) )
  {
    menu->addAction( QObject::tr( "Reset to defaults" ), mWidget, SLOT( resetLayerNodeToDefaults() ) );
    menu->addSeparator();
  }

  QgsLegendStyle::Style currentStyle = QgsLegendRenderer::nodeLegendStyle( mView->currentNode(), mView->layerTreeModel() );

  QList<QgsLegendStyle::Style> lst;
  lst << QgsLegendStyle::Hidden << QgsLegendStyle::Group << QgsLegendStyle::Subgroup;
  for ( QgsLegendStyle::Style style : qgis::as_const( lst ) )
  {
    QAction *action = menu->addAction( QgsLegendStyle::styleLabel( style ), mWidget, SLOT( setCurrentNodeStyleFromAction() ) );
    action->setCheckable( true );
    action->setChecked( currentStyle == style );
    action->setData( ( int ) style );
  }

  return menu;
}
