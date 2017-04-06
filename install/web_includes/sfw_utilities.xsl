<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!-- The elements of this stylesheet support various template calls
       in other stylesheets (like sfw_table.xsl).  These templates can
       be overridden by importing another stylesheet that includes
       templates with the same signature as the one to be replaced.
       
       Like this:

       <xsl:stylesheet ...etc>
          <xsl:import href="sfw_utilties.xsl" />
          <xsl:import href="sfw_utility_overrides.xsl" />
  -->

  <xsl:import href="sfw_generics.xsl" />

  <xsl:variable name="vars" select="/*/*[@rndx][@type='variables']" />
  
  <xsl:template match="*" mode="extra_anchor_attributes">
    <xsl:param name="type" />

    <xsl:attribute name="data-sfw-class">
      <xsl:value-of select="$type" />
    </xsl:attribute>

    <xsl:apply-templates select="." mode="add_on_line_click_attribute" />

  </xsl:template>

  <xsl:template match="*" mode="add_on_line_click_attribute">
    <xsl:variable name="olc_s" select="schema/@on_line_click" />
    <xsl:variable name="olc_r" select="/*[not($olc_s)]/@on_line_click" />
    <xsl:variable name="olc" select="$olc_s|$olc_r" />

    <xsl:if test="$olc">
      <xsl:attribute name="on_line_click">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="$olc" />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>

  <!-- add generic attributes with html- prefix to current element: -->
  <xsl:template match="@*" mode="add_html_attributes">
    <xsl:param name="skip" />
    <xsl:if test="starts-with(local-name(), 'html-')">
      <xsl:variable name="tag" select="substring(local-name(),6)" />
      
      <xsl:variable name="omit">
        <xsl:if test="$skip">
          <xsl:variable name="_tag_" select="concat(' ',$tag,' ')" />
          <xsl:if test="contains($skip, $_tag_)">true</xsl:if>
        </xsl:if>
      </xsl:variable>

      <xsl:if test="not($omit='true')">
        <xsl:attribute name="{$tag}">
          <xsl:value-of select="." />
        </xsl:attribute>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema" mode="show_intro">
    <xsl:param name="class" />
    <xsl:param name="host-type" select="'tr'" />

    <xsl:variable name="i_sch" select="intro" />
    <xsl:variable name="i_doc" select="/*[not($i_sch)]/@intro" />
    <xsl:variable name="intro" select="$i_sch|$i_doc" />

    <xsl:if test="string-length($intro)">
      <xsl:variable name="host_class">
        <xsl:if test="$host-type='tr'">
          <xsl:value-of select="concat(' headfix_', local-name(..),'_',local-name())" />
        </xsl:if>
        <xsl:if test="$class">
          <xsl:value-of select="concat(' ', $class)" />
        </xsl:if>
      </xsl:variable>

      <xsl:choose>
        <xsl:when test="$host-type='tr'">
          <tr class="{$host_class}">
            <td colspan="99">
              <xsl:value-of select="$intro" />
            </td>
          </tr>
        </xsl:when>
        <xsl:otherwise>
          <div class="{$host_class}">
            <xsl:value-of select="$intro" />
          </div>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

  </xsl:template>

  <!-- Recursive template to create button lines.
       Print 1. parent, 2. self, 3. self/buttons -->
  <xsl:template match="*" mode="show_buttons">
    <xsl:param name="class" />
    <xsl:param name="host-type" select="'tr'" />

    <xsl:if test="not(local-name()='buttons')">
      <xsl:variable name="par" select="parent::*" />
      <xsl:if test="$par">
        <xsl:apply-templates select="$par" mode="show_buttons">
          <xsl:with-param name="class" select="$class" />
          <xsl:with-param name="host-type" select="$host-type" />
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>

    <xsl:variable name="host_class">
      <xsl:text>button_row</xsl:text>
      <xsl:if test="$host-type='tr'">
        <xsl:value-of select="concat(' headfix_', local-name(..),'_',local-name())" />
      </xsl:if>
      <xsl:value-of select="concat(' sfwid_', generate-id())" />
      <xsl:if test="$class">
        <xsl:value-of select="concat(' ', $class)" />
      </xsl:if>
    </xsl:variable>
    
    <xsl:variable name="buttons" select="button" />

    <xsl:if test="count($buttons)&gt;0">
      <xsl:choose>
        <xsl:when test="$host-type='tr'">
          <tr class="{$host_class}">
            <td colspan="99" style="background-color #66FF66">
              <xsl:apply-templates select="$buttons" mode="show" />
            </td>
          </tr>
        </xsl:when>
        <xsl:otherwise>
          <p class="{$host_class}">
            <xsl:apply-templates select="$buttons" mode="show" />
          </p>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <xsl:if test="not(local-name()='buttons')">
      <xsl:apply-templates select="buttons" mode="show_buttons">
        <xsl:with-param name="class" select="$class" />
        <xsl:with-param name="host-type" select="$host-type" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <!--
      This template produces a string from the "str" param with tokens
      replaced with values included in document.
  -->
  <xsl:template name="resolve_refs">
    <xsl:param name="str" />

    <xsl:variable name="vv" select="substring($str,1)" />

    <xsl:variable name="delim">
      <xsl:variable name="after" select="substring(substring-after($str,'{'),1,1)" />
      <!-- <xsl:if test="$after='@' or $after='$'"> -->
      <xsl:if test="$after and contains('@$!', $after)">
        <xsl:value-of select="concat('{',$after)" />
      </xsl:if>
    </xsl:variable>

    <xsl:variable name="before">
      <xsl:if test="$delim">
        <xsl:value-of select="substring-before($str, $delim)" />
      </xsl:if>
    </xsl:variable>

    <xsl:variable name ="len_before" select="string-length($before)" />

    <xsl:variable name="ref">
      <xsl:if test="starts-with($str,$delim)">
        <xsl:variable name="end" select="substring-before($str,'}')" />
        <xsl:if test="$end and not (contains($end,' '))">
          <xsl:value-of select="substring($end, 3)" />
        </xsl:if>
      </xsl:if>
    </xsl:variable>
    
    <xsl:variable name="val">
      <xsl:choose>
        <xsl:when test="$len_before">
          <xsl:value-of select="$before" />
        </xsl:when>
        <xsl:when test="string-length($ref)">
          <xsl:variable name="type" select="substring($str,2,1)" />
          <xsl:choose>
            <xsl:when test="$type='$'">
              <xsl:call-template name="get_var_value">
                <xsl:with-param name="name" select="$ref" />
              </xsl:call-template>
            </xsl:when>
            <xsl:when test="$type='@'">
              <xsl:call-template name="get_data_value">
                <xsl:with-param name="name" select="$ref" />
              </xsl:call-template>
            </xsl:when>
            <xsl:when test="$type='!'">
              <xsl:value-of select="concat($delim, $ref, '}')" />
            </xsl:when>
          </xsl:choose>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$str" /></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:value-of select="$val" />
    
    <xsl:variable name="skiplen">
      <xsl:choose>
        <xsl:when test="string-length($ref) &gt; 0">
          <xsl:value-of select="string-length($ref)+4" />
        </xsl:when>
        <xsl:when test="$len_before">
          <xsl:value-of select="$len_before+1" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="string-length($str) &gt;= $skiplen">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="substring($str, $skiplen)" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="get_data_value">
    <xsl:param name="name" />
    <xsl:variable name="results" select="/*/*[@rndx and not(@type='variables')]" />
    <xsl:variable name="fresults"
                  select="$results[count(*[local-name()!='schema'])=1]" />
    <xsl:variable name="rows" select="$fresults/*[local-name()!='schema']" />
    <xsl:if test="count($rows)">
      <xsl:variable name="attrs" select="$rows/@*[local-name()=$name]" />
      <xsl:if test="count($attrs)">
        <xsl:value-of select="$attrs[1]" />
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*[@rndx][@type='variables']" mode="get_var_value">
    <xsl:param name="name" />
    <xsl:variable name="row" select="*[local-name()=current()/@row-name]" />
    <xsl:if test="$row">
      <xsl:variable name="val" select="$row/@*[local-name()=$name]" />
      <xsl:if test="$val">
        <xsl:value-of select="$val" />
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template name="get_var_value">
    <xsl:param name="name" />
    <xsl:apply-templates select="$vars" mode="get_var_value">
      <xsl:with-param name="name" select="$name" />
    </xsl:apply-templates>
  </xsl:template>

</xsl:stylesheet>
