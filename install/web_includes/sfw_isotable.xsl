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
      <xsl:apply-templates select="." mode="isotable_colhead" />
    </tr>
  </xsl:template>

  <xsl:template match="field" mode="isotable_cell">
    <xsl:param name="row" />
    <xsl:variable name="value" select="$row/@*[local-name()=current()/@name]" />
    <td><xsl:value-of select="$value" /></td>
  </xsl:template>

  <xsl:template match="*" mode="isotable_row">
    <xsl:param name="schema" />

    <tr>
      <xsl:apply-templates select="$schema/field" mode="isotable_cell">
        <xsl:with-param name="row" select="." />
      </xsl:apply-templates>
    </tr>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="construct_isotable">
    <table>
      <thead>
        <xsl:apply-templates select="schema" mode="isotable_head" />
      </thead>
      <tbody>
        <xsl:apply-templates
            select="*[local-name()=../@row-name]"
            mode="isotable_row">
          <xsl:with-param name="schema" select="schema" />
        </xsl:apply-templates>
      </tbody>
    </table>
  </xsl:template>

  <xsl:template match="*[@rndx][not(schema)]" mode="construct_isotable">
    <div>No isotable without a schema in the target result.</div>
  </xsl:template>

  <!-- Above this line, the templates support the following templates. -->

  <!-- The following templates are directly called by the framework. -->
  <xsl:template match="field[@type='isotable'][@result]" mode="construct_input">
    <xsl:param name="data" />
    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <div>Ha! in the construct_input</div>
    <div class="isotable" data-sfw-class="isotable" data-sfw-input="true" name="@name">
      <xsl:apply-templates select="$result" mode="construct_isotable" />
    </div>
  </xsl:template>

  <xsl:template match="field[@type='isotable']" mode="construct_form_input">
    <xsl:param name="data" />
    <xsl:param name="result-schema" select="../*" />
    <xsl:param name="view-mode" select="/.." />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <div class="isotable" data-sfw-class="isotable" data-sfw-input="true" name="@name">
      <xsl:apply-templates select="$result" mode="construct_isotable" />
    </div>

  </xsl:template>

  <!-- Fall-through template that announces an error. -->
  <xsl:template match="field[@type='isotable'][not(@result)]" mode="construct_input">
    <div>ISOTable field type requires a result link to row data.</div>
  </xsl:template>


</xsl:stylesheet>
