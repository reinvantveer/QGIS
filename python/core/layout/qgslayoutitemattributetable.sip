/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemattributetable.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutItemAttributeTable: QgsLayoutTable
{
%Docstring
 A layout table subclass that displays attributes from a vector layer.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemattributetable.h"
%End
  public:

    enum ContentSource
    {
      LayerAttributes,
      AtlasFeature,
      RelationChildren
    };

    QgsLayoutItemAttributeTable( QgsLayout *layout /TransferThis/ );
%Docstring
 Constructor for QgsLayoutItemAttributeTable, attached to the specified ``layout``.

 Ownership is transferred to the layout.
%End

    virtual int type() const;

    virtual QString stringType() const;

    virtual QString displayName() const;


    static QgsLayoutItemAttributeTable *create( QgsLayout *layout ) /Factory/;
%Docstring
 Returns a new QgsLayoutItemAttributeTable for the specified parent ``layout``.
 :rtype: QgsLayoutItemAttributeTable
%End

    void setSource( ContentSource source );
%Docstring
 Sets the ``source`` for attributes to show in table body.
.. seealso:: source()
%End

    ContentSource source() const;
%Docstring
 Returns the source for attributes shown in the table body.
.. seealso:: setSource()
 :rtype: ContentSource
%End

    QgsVectorLayer *sourceLayer();
%Docstring
 Returns the source layer for the table, considering the table source mode. For example,
 if the table is set to atlas feature mode, then the source layer will be the
 atlas coverage layer. If the table is set to layer attributes mode, then
 the source layer will be the user specified vector layer.
 :rtype: QgsVectorLayer
%End

    void setVectorLayer( QgsVectorLayer *layer );
%Docstring
 Sets the vector ``layer`` from which to display feature attributes.

 This is only considered if the table source() is LayerAttributes.

.. seealso:: vectorLayer()
%End

    QgsVectorLayer *vectorLayer() const;
%Docstring
 Returns the vector layer the attribute table is currently using.

 This is only considered if the table source() is LayerAttributes.

.. seealso:: setVectorLayer()
.. seealso:: sourceLayer()
 :rtype: QgsVectorLayer
%End

    void setRelationId( const QString &id );
%Docstring
 Sets the relation ``id`` from which to display child features
.. seealso:: relationId()
.. seealso:: setSource()
.. note::

   Only used if table source() is set to RelationChildren.
%End

    QString relationId() const;
%Docstring
 Returns the relation id which the table displays child features from.
.. seealso:: setRelationId()
.. seealso:: source()
.. note::

   Only used if table source() is set to RelationChildren.
 :rtype: str
%End

    void resetColumns();
%Docstring
 Resets the attribute table's columns to match the vector layer's fields.
.. seealso:: setVectorLayer()
%End

    void setMap( QgsLayoutItemMap *map );
%Docstring
 Sets a layout ``map`` to use to limit the extent of features shown in the
 attribute table. This setting only has an effect if setDisplayOnlyVisibleFeatures is
 set to true. Changing the map forces the table to refetch features from its
 vector layer, and may result in the table changing size to accommodate the new displayed
 feature attributes.
.. seealso:: map()
.. seealso:: setDisplayOnlyVisibleFeatures
%End

    QgsLayoutItemMap *map() const;
%Docstring
 Returns the layout map whose extents are controlling the features shown in the
 table. The extents of the map are only used if displayOnlyVisibleFeatures() is true.
.. seealso:: setMap()
.. seealso:: displayOnlyVisibleFeatures()
 :rtype: QgsLayoutItemMap
%End

    void setMaximumNumberOfFeatures( int features );
%Docstring
 Sets the maximum number of ``features`` shown by the table. Changing this setting may result
 in the attribute table changing its size to accommodate the new number of rows, and requires
 the table to refetch features from its vector layer.
.. seealso:: maximumNumberOfFeatures()
%End

    int maximumNumberOfFeatures() const;
%Docstring
 Returns the maximum number of features to be shown by the table.
.. seealso:: setMaximumNumberOfFeatures()
 :rtype: int
%End

    void setUniqueRowsOnly( bool uniqueOnly );
%Docstring
 Sets attribute table to only show unique rows.

 Set ``uniqueOnly`` to true to show only unique rows. Duplicate rows
 will be stripped from the table.

.. seealso:: uniqueRowsOnly()
%End

    bool uniqueRowsOnly() const;
%Docstring
 Returns true if the table is set to show only unique rows.

.. seealso:: setUniqueRowsOnly()
 :rtype: bool
%End

    void setDisplayOnlyVisibleFeatures( bool visibleOnly );
%Docstring
 Sets the attribute table to only show features which are visible in a map item. Changing
 this setting forces the table to refetch features from its vector layer, and may result in
 the table changing size to accommodate the new displayed feature attributes.

.. seealso:: displayOnlyVisibleFeatures()
.. seealso:: setMap()
%End

    bool displayOnlyVisibleFeatures() const;
%Docstring
 Returns true if the table is set to show only features visible on a corresponding
 map item.

.. seealso:: map()
.. seealso:: setDisplayOnlyVisibleFeatures()
 :rtype: bool
%End

    void setFilterToAtlasFeature( bool filterToAtlas );
%Docstring
 Sets attribute table to only show features which intersect the current atlas
 feature.

.. seealso:: filterToAtlasFeature()
%End

    bool filterToAtlasFeature() const;
%Docstring
 Returns true if the table is set to only show features which intersect the current atlas
 feature.

.. seealso:: setFilterToAtlasFeature()
 :rtype: bool
%End

    bool filterFeatures() const;
%Docstring
 Returns true if a feature filter is active on the attribute table.

.. seealso:: setFilterFeatures()
.. seealso:: featureFilter()
 :rtype: bool
%End

    void setFilterFeatures( bool filter );
%Docstring
 Sets whether the feature filter is active for the attribute table. Changing
 this setting forces the table to refetch features from its vector layer, and may result in
 the table changing size to accommodate the new displayed feature attributes.
.. seealso:: filterFeatures()
.. seealso:: setFeatureFilter()
%End

    QString featureFilter() const;
%Docstring
 Returns the current expression used to filter features for the table. The filter is only
 active if filterFeatures() is true.

.. seealso:: setFeatureFilter()
.. seealso:: filterFeatures()
 :rtype: str
%End

    void setFeatureFilter( const QString &expression );
%Docstring
 Sets the ``expression`` used for filtering features in the table. The filter is only
 active if filterFeatures() is set to true. Changing this setting forces the table
 to refetch features from its vector layer, and may result in
 the table changing size to accommodate the new displayed feature attributes.

.. seealso:: featureFilter()
.. seealso:: setFilterFeatures()
%End

    void setDisplayedFields( const QStringList &fields, bool refresh = true );
%Docstring
 Sets the attributes to display in the table.
 \param fields list of fields names from the vector layer to show.
 Set to an empty list to show all feature attributes.
 \param refresh set to true to force the table to refetch features from its vector layer
 and immediately update the display of the table. This may result in the table changing size
 to accommodate the new displayed feature attributes.
%End


    void setWrapString( const QString &wrapString );
%Docstring
 Sets a string to wrap the contents of the table cells by. Occurrences of this string will
 be replaced by a line break.
 \param wrapString string to replace with line break
.. seealso:: wrapString()
%End

    QString wrapString() const;
%Docstring
 Returns the string used to wrap the contents of the table cells by. Occurrences of this string will
 be replaced by a line break.
.. seealso:: setWrapString()
 :rtype: str
%End


    virtual QgsExpressionContext createExpressionContext() const;


  protected:

    virtual bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemattributetable.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
