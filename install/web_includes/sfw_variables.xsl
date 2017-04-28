<?xml version="1.0" encoding="utf-8" ?>

<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:variable name="result-row"
                select="/*[@mode-type='form-result']/*[@rndx=1]/*[@error]" />

  <xsl:variable name="docel_msg" select="/message" />
  <xsl:variable name="child_msg" select="/*[not($docel_msg)]/message" />
  <xsl:variable name="msg-el" select="$docel_msg | $child_msg" />

  <xsl:variable name="err_condition">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">1</xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="gsview" select="/*/views/view[@selected]" />
  <xsl:variable name="gfview" select="/*[not($gsview)]/views/view[1]" />
  <xsl:variable name="gview" select="$gsview | $gfview" />

  <xsl:variable name="view_name" select="$gview/@name" />

  <xsl:variable name="view_result" select="/*/*[@rndx][local-name()=$gview/@result]"/>
  <xsl:variable name="first_result" select="/*/*[not($view_result)][@rndx=1]" />

  <xsl:variable name="gresult" select="$view_result | $first_result" />

  <xsl:variable name="gschema" select="/*/schema" />

</xsl:stylesheet>
