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
  <xsl:variable name="jslist_sfw_debug">classes dpicker Events Dialog Moveable XML sfw_0 sfw_doc sfw_tbase sfw_calendar sfw_debug sfw_dom sfw_form sfw_form_view sfw_ulselect sfw_mixed_view sfw_selectx sfw_onload sfw_table sfw_assoc sfw_lookup sfw_pwtoggle sfw_isotable sfw_iltable</xsl:variable>

  <!-- Main entry point to template in this stylesheet. -->
  <xsl:template match="/*" mode="construct_scripts">
    <!-- Param to switch between javascript files to include:
         the value 'debug' uses debug scripts, anything else
         uses sfw.min.js -->
    <xsl:param name="jscripts" />

    <xsl:variable name="doc_vars" select="@*" />
    <xsl:variable name="result_vars" select="*[@rndx][@type='variable']/@*" />
    <xsl:variable name="all_vars" select="$doc_vars | $result_vars" />

    <!-- The default behavior, modified by the jscripts param, is to use 'debug' scripts
         because sfw.min.js requires that a minimizer/uglifier is installed and used to
         generate sfw.min.js. -->
    <xsl:call-template name="add_js">
      <xsl:with-param name="path">includes/</xsl:with-param>
      <xsl:with-param name="list">
        <xsl:choose>
          <xsl:when test="$jscripts='debug'"><xsl:value-of select="$jslist_sfw_debug" /></xsl:when>
          <xsl:otherwise>sfw.min</xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
    </xsl:call-template>

    <xsl:if test="@javascript">
      <xsl:call-template name="add_js">
        <xsl:with-param name="list" select="@javascript" />
      </xsl:call-template>
    </xsl:if>
    <xsl:value-of select="$nl" />

    <xsl:if test="count($all_vars)">
      <script>
        <xsl:value-of select="concat('var ', $vars_obj, '={};', $nl)" />
        <xsl:apply-templates select="$all_vars" mode="construct_attribute_variable" />
      </script>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@*" mode="construct_attribute_variable">
    <xsl:variable name="n" select="translate(local-name(),'-','_')" />
    <xsl:value-of
        select="concat($vars_obj, '.', $n, ' = &quot;', ., '&quot;;', $nl)" />
  </xsl:template>

</xsl:stylesheet>
