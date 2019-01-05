<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_generics.xsl" />
  <xsl:import href="sfw_utilities.xsl" />
  <xsl:import href="sfw_schema.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>

  <xsl:template match="field" mode="isotable_colhead">
    <xsl:variable name="label">
      <xsl:apply-templates select="." mode="get_label" />
    </xsl:variable>
    <th><xsl:value-of select="$label" /></th>
  </xsl:template>

  <xsl:template match="schema" mode="isotable_head">
    <tr>
      <th />
      <xsl:apply-templates select="field[not(@hidden)]" mode="isotable_colhead" />
    </tr>
  </xsl:template>

  <xsl:template match="field" mode="isotable_cell">
    <xsl:param name="row" />
    <td>
      <xsl:if test="$row">
        <xsl:value-of select="$row/@*[local-name()=current()/@name]" />
      </xsl:if>
    </td>
  </xsl:template>

  <xsl:template match="field" mode="isotable_field">
    <xsl:param name="row" />
    <td class="field">
      <xsl:apply-templates select="." mode="construct_input" />
    </td>
  </xsl:template>

  <xsl:template match="*" mode="isotable_remove_button">
    <xsl:param name="schema" />
    <xsl:variable name="idattr" select="$schema/field[@primary-key]/@name" />
    <xsl:variable name="rowid" select="position()" />
    <xsl:if test="$idattr">
      <xsl:variable name="idval" select="@*[local-name()=$idattr]" />
      <img class="action" title="remove item" src="includes/remove-item.png" data-id="{$idval}" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="isotable_row">
    <xsl:param name="schema" />
    <xsl:variable name="pos" select="position()" />
    <tr data-pos="{$pos}">
      <td>
        <img class="action" title="remove item" src="includes/remove-item.png" />
      </td>
      <xsl:apply-templates select="$schema/field[not(@hidden)]" mode="isotable_cell">
        <xsl:with-param name="row" select="." />
      </xsl:apply-templates>
    </tr>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="isotable_fill_tbody">
    <xsl:variable name="rows" select="*[local-name()=../@row-name]" />

    <xsl:apply-templates select="$rows" mode="isotable_row">
      <xsl:with-param name="schema" select="schema" />
    </xsl:apply-templates>

    <tr class="isotable_form">
      <td><img class="action" title="add item" src="includes/add-item.png" /></td>
      <xsl:apply-templates select="schema/field[not(@hidden)]" mode="isotable_field" />
    </tr>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="construct_isotable">
    <table>
      <thead>
        <xsl:apply-templates select="schema" mode="isotable_head" />
      </thead>
      <tbody>
        <xsl:apply-templates select="." mode="isotable_fill_tbody" />
      </tbody>
    </table>
  </xsl:template>

  <xsl:template match="*[@rndx][not(schema)]" mode="construct_isotable">
    <div>No isotable without a schema in the target result.</div>
  </xsl:template>

  <xsl:template match="field" mode="serialize_isotable_cell">
    <xsl:param name="row" />
    <xsl:variable name="attr" select="$row/@*[local-name()=current()/@name]" />
    <xsl:if test="position() &gt; 1"><xsl:text>|</xsl:text></xsl:if>
    <xsl:if test="$attr and string-length($attr)">
      <xsl:value-of select="$attr" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="serialize_isotable_row">
    <xsl:if test="position() &gt; 1"><xsl:text>;</xsl:text></xsl:if>

    <xsl:variable name="schema" select="../schema" />
    <xsl:apply-templates select="$schema/field" mode="serialize_isotable_cell">
      <xsl:with-param name="row" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="serialize_isotable">
    <xsl:apply-templates select="*[local-name()=../@row-name]" mode="serialize_isotable_row" />
  </xsl:template>

  <xsl:template match="field" mode="phantom_isotable_input">
    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <xsl:element name="input">
      <xsl:attribute name="type">hidden</xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
      <xsl:attribute name="value">
        <xsl:apply-templates select="$result" mode="serialize_isotable" />
      </xsl:attribute>
    </xsl:element>
  </xsl:template>

  <!-- Above this line, the templates support the following templates. -->

  <!-- The following templates are directly called by the framework. -->
  <xsl:template match="field[@type='isotable'][@result]" mode="construct_input">
    <xsl:param name="data" />
    <xsl:param name="result-schema" select="/.." />
    <xsl:param name="view-mode" select="/.." />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />

    <xsl:apply-templates select="." mode="phantom_isotable_input" />
    <form class="isotable"
          data-sfw-class="isotable"
          data-sfw-input="true"
          name="{@name}">
      <xsl:apply-templates select="$result" mode="construct_isotable" />
    </form>
  </xsl:template>

  <!-- Fall-through template that announces an error. -->
  <xsl:template match="field[@type='isotable'][not(@result)]" mode="construct_input">
    <div>ISOTable field type requires a result link to row data.</div>
  </xsl:template>


  <!-- Modeless match for replotting the table contents (tbody). -->
  <xsl:template match="*[@rndx][@iso_replot='table']">
    <xsl:apply-templates select="." mode="isotable_fill_tbody" />
  </xsl:template>

  <xsl:template match="*[@rndx][@iso_replot='value']">
    <xsl:apply-templates select="." mode="serialize_isotable" />
  </xsl:template>

  <!-- TWO Modeless matches for creating a form -->
  <!-- matches a row, so type is not new -->
  <xsl:template match="*[parent::*[@rndx][@iso_replot='form'][schema]][local-name()=../@row-name]">
    <xsl:apply-templates select="../schema" mode="construct_form">
      <xsl:with-param name="prow" select="." />
      <xsl:with-param name="type" select="'isotable_form'" />
    </xsl:apply-templates>
  </xsl:template>

  <!-- matching a result, not an existing row, type is new. -->
  <xsl:template match="*[@rndx][@iso_replot='form'][schema]">
    <xsl:apply-templates select="schema" mode="construct_form">
      <xsl:with-param name="type" select="'isotable_form'" />
      <xsl:with-param name="prow" select="/" />
    </xsl:apply-templates>
  </xsl:template>

</xsl:stylesheet>
