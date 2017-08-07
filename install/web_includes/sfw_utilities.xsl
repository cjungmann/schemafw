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
  
  <xsl:template match="*[@rndx]" mode="add_data_attribute">
    <xsl:param name="name" />
    <xsl:variable name="s_val" select="schema/@*[local-name()=$name]"/>
    <xsl:variable name="r_val" select="@*[not($s_val)][local-name()=$name]"/>
    <xsl:variable name="d_val"
                  select="/*[not($s_val|$r_val)]/@*[local-name()=$name]"/>
    <xsl:variable name="val" select="$d_val | $r_val | $s_val" />

    <xsl:if test="$val">
      <xsl:variable name="aname" select="concat('data-',$name)" />
      <xsl:attribute name="{$aname}">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="$val" />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>

  <xsl:template name="build_sfw_class_attribute">
    <xsl:param name="schema" select="/.." />
    <xsl:attribute name="data-sfw-class">
      <xsl:choose>
        <xsl:when test="$schema and $schema/@type">
          <xsl:value-of select="@type" />
        </xsl:when>
        <xsl:when test="$gview and $gview/@type">
          <xsl:value-of select="$gview/@type" />
        </xsl:when>
        <xsl:when test="$mode-type">
          <xsl:value-of select="$mode-type" />
        </xsl:when>
        <xsl:otherwise>table</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="schema" mode="add_sfw_class_attribute">
    <xsl:call-template name="build_sfw_class_attribute">
      <xsl:with-param name="schema" select="." />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="add_sfw_class_attribute">
    <xsl:call-template name="build_sfw_class_attribute">
      <xsl:with-param name="schema" select="@schema" />
    </xsl:call-template>
  </xsl:template>

  <!-- add generic attributes with html- prefix to current element: -->
  <xsl:template match="@*" mode="add_html_attribute">
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

  <xsl:template match="button/@*" mode="add_button_attribute">
    <xsl:param name="skip-data-prefix" />

    <xsl:variable name="tname" select="concat(' ',local-name(),' ')" />
    <xsl:variable name="skip-prefix" select="contains($skip-data-prefix,$tname)" />

    <xsl:variable name="name">
      <xsl:choose>
        <xsl:when test="$skip-prefix">
          <xsl:value-of select="local-name()" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat('data-', local-name())" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <xsl:attribute name="{$name}">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="." />
      </xsl:call-template>
      <!-- <xsl:value-of select="." /> -->
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="@*" mode="make_jump_link">
    <p>Click
    <xsl:element name="a">
      <xsl:attribute name="href"><xsl:value-of select="." /></xsl:attribute>
    </xsl:element>
    if you are not taken there.
    </p>
  </xsl:template>

  <xsl:template match="button" mode="skip_check">0</xsl:template>

  <xsl:template match="button" mode="construct_button">
    <xsl:variable name="skip">
      <xsl:apply-templates select="." mode="skip_check" />
    </xsl:variable>

    <xsl:if test="not(number($skip))">
      <xsl:element name="button">
        <xsl:attribute name="type">button</xsl:attribute>
        
        <xsl:apply-templates select="@*[not(local-name()='label')]"
                             mode="add_button_attribute">
          <xsl:with-param name="skip-data-prefix" select="' name value disabled '" />
        </xsl:apply-templates>
        
        <xsl:value-of select="@label" />
      </xsl:element>
    </xsl:if>
  </xsl:template>

  <xsl:template name="construct_title">
    <xsl:param name="str" />
    <h2>
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="$str" />
      </xsl:call-template>
    </h2>
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

  <xsl:template match="@*" mode="resolve_refs">
    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str" select="." />
    </xsl:call-template>
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

