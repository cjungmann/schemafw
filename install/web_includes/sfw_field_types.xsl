<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:html="http://www.w3.org/1999/xhtml"
    exclude-result-prefixes="html">

  <xsl:output method="xml"
              doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
              version="1.0"
              indent="yes"
              omit-xml-declaration="yes"
              encoding="UTF-8"/>

  <!-- Each new field type should include at least two template modes:
       - mode="get_value" for table cells (and other view-only applications),
       - mode="construct_input" for an editable form field
       Other supporting templates following the two interface templates.
       I suggest the following order:
       - The two main interface templates,
       - Other supporting templates
       - Error templates for modes get_value and construct_input for fields
         that match the type but will fail to render.
         ** Reminder ** XSL applys the last matching template found.
-->

  <xsl:template match="field[@type='select_result'][@result]" mode="get_value">
    <xsl:param name="data" />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <xsl:variable name="schema" select="$result/schema" />

    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:variable name="dvalue" select="$data/@*[local-name()=$name]" />

    <xsl:variable name="id_name">
      <xsl:apply-templates select="$result/schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="show_name">
      <xsl:apply-templates select="." mode="get_display_field">
        <xsl:with-param name="schema" select="$schema" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable
        name="row"
        select="$result/*[local-name()=../@row-name][@*[local-name()=$id_name]=$dvalue]" />

    <xsl:choose>
      <xsl:when test="$row">
        <xsl:value-of select="$row/@*[local-name()=$show_name]" />
      </xsl:when>
      <xsl:when test="@show and string-length($show_name)=0">
        Field '<xsl:value-of select="@show" />' not found in schema
      </xsl:when>
    </xsl:choose>

  </xsl:template>

  <xsl:template match="field[@type='select_result'][@result]" mode="construct_input">
    <xsl:param name="data" />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <xsl:variable name="schema" select="$result/schema" />

    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:variable name="dvalue" select="$data/@*[local-name()=$name]" />

    <xsl:variable name="id_name">
      <xsl:apply-templates select="$result/schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="show_name">
      <xsl:apply-templates select="." mode="get_display_field">
        <xsl:with-param name="schema" select="$schema" />
      </xsl:apply-templates>
    </xsl:variable>

    <select name="{$name}">
      <xsl:apply-templates select="$result" mode="select_result_options">
        <xsl:with-param name="value" select="$dvalue" />
        <xsl:with-param name="id_name" select="$id_name" />
        <xsl:with-param name="show_name" select="$show_name" />
      </xsl:apply-templates>
    </select>
  </xsl:template>

  <xsl:template match="field[@type='select_result'][not(@result)]" mode="get_value">
    Type select_result/get_value missing result attribute
  </xsl:template>
  <xsl:template match="field[@type='select_result'][not(@result)]" mode="construct_input">
    Type select_result/get_value missing result attribute
  </xsl:template>

  <xsl:template match="field[@type='select_result']" mode="get_display_field">
    <xsl:param name="schema" />
    <xsl:choose>
      <xsl:when test="@show">
        <xsl:value-of select="@show" />
      </xsl:when>
      <xsl:when test="count($result/schema/field) &gt; 1">
        <xsl:value-of select="$result/schema/field[local-name()!=$id_name][1]/@name" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$id_name" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="select_result_options">
    <xsl:param name="value" />
    <xsl:param name="id_name" />
    <xsl:param name="show_name" />

    <xsl:variable name="rows" select="*[local-name()=../@row-name]" />
    <div><xsl:value-of select="concat(count($rows), ' rows in the result.')" /></div>
    
    <xsl:for-each select="$rows">
      <xsl:variable name="v" select="@*[local-name()=$id_name]" />
      <xsl:element name="option">
        <xsl:attribute name="value">
          <xsl:value-of select="$v" />
        </xsl:attribute>
        <xsl:if test="$value=$v">
          <xsl:attribute name="selected">selected</xsl:attribute>
        </xsl:if>
        <xsl:value-of select="@*[local-name()=$show_name]" />
      </xsl:element>
    </xsl:for-each>
    
    
  </xsl:template>
  

</xsl:stylesheet>
