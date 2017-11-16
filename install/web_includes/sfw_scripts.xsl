<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!-- for resolve_refs -->
  <xsl:import href="sfw_utilities.xsl" />

  <xsl:variable name="vars_obj" select="'sfwvars'" />
  
  <xsl:variable name="jslist_sfw_brief">sfw</xsl:variable>
  <xsl:variable name="jslist_sfw_minified">sfw.min</xsl:variable>
  <xsl:variable name="jslist_sfw_debug">sfw_0 sfw_tbase sfw_calendar sfw_debug sfw_dom sfw_form sfw_ulselect sfw_mixed_view sfw_onload sfw_table sfw_assoc sfw_iltable</xsl:variable>
  
  <!-- Set jslist_sfw to one of the above variables for the desired script-set. -->
  <xsl:variable name="jslist_sfw" select="$jslist_sfw_debug" /> 

  <xsl:variable name="jslist_utils">classes dpicker Events Dialog Moveable XML</xsl:variable>

  <!-- Main entry point to template in this stylesheet. -->
  <xsl:template match="/*" mode="construct_scripts">
    <xsl:variable name="doc_vars" select="@*" />
    <xsl:variable name="result_vars" select="*[@rndx][@type='variable']/@*" />
    <xsl:variable name="all_vars" select="$doc_vars | $result_vars" />

    <xsl:call-template name="construct_sfw_scripts" />
    <xsl:if test="count($all_vars)">
      <script>
        <xsl:value-of select="concat('var ', $vars_obj, '={};', $nl)" />
        <xsl:apply-templates select="$all_vars" mode="construct_attribute_variable" />
      </script>
    </xsl:if>
  </xsl:template>

  <xsl:template name="construct_sfw_scripts">
    <xsl:call-template name="add_js">
      <xsl:with-param name="list" select="$jslist_sfw" />
    </xsl:call-template>
    <xsl:call-template name="add_js">
      <xsl:with-param name="list" select="$jslist_utils" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="@*" mode="construct_attribute_variable">
    <xsl:variable name="n" select="translate(local-name(),'-','_')" />
    <xsl:value-of
        select="concat($vars_obj, '.', $n, ' = &quot;', ., '&quot;;', $nl)" />
  </xsl:template>

  <!-- Add variables from set in the main stylesheet. -->
  <xsl:template match="/*" mode="add_document_vars">
    <xsl:if test="count($vars)">
      <script>var <xsl:value-of select="$vars_obj" />={};</script>
      <xsl:value-of select="$nl" />
      <xsl:apply-templates select="." mode="add_document_variables" />
    </xsl:if>
  </xsl:template>


</xsl:stylesheet>
