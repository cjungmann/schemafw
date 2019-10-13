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
  <xsl:import href="sfw_variables.xsl" />
  <xsl:import href="sfw_resolver.xsl" />

  <xsl:template match="field" mode="classify">
    <xsl:choose>
      <xsl:when test="@data-class"><xsl:value-of select="@data-class" /></xsl:when>
      <xsl:when test="substring(@type,(string-length(@type)-3))='INT' or @type='INTEGER'">INT_CLASS</xsl:when>
      <xsl:when test="@type='DECIMAL' or @type='NUMERIC'">DEC_CLASS</xsl:when>
      <xsl:when test="contains(@type,'TIME') or contains(@type,'DATE')">TIME_CLASS</xsl:when>
      <xsl:when test="@type='FLOAT' or @type='DOUBLE'">FLOAT_CLASS</xsl:when>
      <xsl:otherwise>STRING_CLASS</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="field" mode="def_css_class">
    <xsl:variable name="class">
      <xsl:apply-templates select="." mode="classify" />
    </xsl:variable>
    <xsl:choose>
      <xsl:when
          test="$class='INT_CLASS' or $class='DEC_CLASS' or $class='FLOAT_CLASS'">def_right</xsl:when>
      <xsl:when test="$class='TIME_CLASS'">def_center</xsl:when>
      <xsl:otherwise>def_left</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="get_idname">

    <xsl:variable name="fname">
      <xsl:apply-templates select="schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="attr" select="*[../@row-name][1]/@*[1]" />

    <xsl:choose>
      <xsl:when test="schema"><xsl:value-of select="$fname" /></xsl:when>
      <xsl:when test="$attr"><xsl:value-of select="local-name($attr)" /></xsl:when>
      <xsl:otherwise>id</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="get_idval">
    <xsl:param name="row" />
    <xsl:variable name="idname">
      <xsl:apply-templates select="." mode="get_idname" />
    </xsl:variable>
    <xsl:value-of select="$row/@*[local-name()=$idname]" />
  </xsl:template>


  <xsl:template match="@*" mode="gen_path">
    <xsl:value-of select="concat('[@', local-name(), '=', $apos, ., $apos, ']')" />
  </xsl:template>

  <xsl:template match="*" mode="gen_path">
    <xsl:variable name="par" select="parent::*" />
    <xsl:if test="$par">
      <xsl:apply-templates select="$par" mode="gen_path" />
    </xsl:if>
    <xsl:value-of select="concat('/', local-name())" />
  </xsl:template>

  <xsl:template match="*[local-name()=../@row-name]" mode="gen_path">
    <xsl:variable name="par" select="parent::*" />
    <xsl:if test="$par">
      <xsl:apply-templates select="$par" mode="gen_path" />
    </xsl:if>

    <xsl:variable name="idname">
      <xsl:apply-templates select=".." mode="get_idname" />
    </xsl:variable>
    <xsl:value-of select="concat('/', local-name())" />
    <xsl:apply-templates select="@*[local-name()=$idname]" mode="gen_path" />
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="gen_path">
    <xsl:variable name="par" select="parent::*" />
    <xsl:if test="$par">
      <xsl:apply-templates select="$par" mode="gen_path" />
    </xsl:if>
    <xsl:value-of select="concat('/', local-name())" />
    <xsl:apply-templates select="@rndx" mode="gen_path" />
    <xsl:apply-templates select="@merged" mode="gen_path" />
  </xsl:template>

  <xsl:template match="*" mode="add_result_attribute">
    <xsl:variable name="result_inst" select="ancestor-or-self::*[@result]/@result" />
    <xsl:variable name="result" select="ancestor-or-self::*[@rndx]" />

    <xsl:variable name="val">
      <xsl:choose>
        <xsl:when test="$result_inst">
          <xsl:value-of select="$result_inst[1]" />
        </xsl:when>
        <xsl:when test="$result">
          <xsl:apply-templates select="$result" mode="gen_path" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="$val">
      <xsl:attribute name="data-result"><xsl:value-of select="$val" /></xsl:attribute>
    </xsl:if>

  </xsl:template>
  
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

  <xsl:template match="@*" mode="add_resolved_attribute">
    <xsl:variable name="name" select="local-name()" />
    <xsl:attribute name="{$name}">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="." />
      </xsl:call-template>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="@*" mode="add_resolved_data_attribute">
    <xsl:variable name="name" select="concat('data-', local-name())" />
    <xsl:attribute name="{$name}">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="." />
      </xsl:call-template>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="@*" mode="resolve_url">
    <xsl:variable name="excl" select="number(substring(.,1,1)='!')" />
    <xsl:variable name="ques" select="number(substring(.,($excl+1),1)='?')" />
    <xsl:variable name="strpos" select="(1 + $excl)" />

    <xsl:variable name="pre">
      <xsl:if test="$ques='1'">
        <xsl:value-of select="ancestor-or-self::*/@script" />
      </xsl:if>
    </xsl:variable>

    <xsl:if test="$excl='1'">!</xsl:if>
    <xsl:value-of select="$pre" />

    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str" select="substring(.,$strpos)" />
      <xsl:with-param name="escape" select="1" />
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="@*" mode="add_url_attribute">
    <xsl:variable name="aname" select="concat('data-url-',local-name())" />
    <xsl:attribute name="{$aname}">
      <xsl:apply-templates select="." mode="resolve_url" />
    </xsl:attribute>
  </xsl:template>

  <xsl:template name="build_sfw_class_attribute">
    <xsl:param name="schema" select="/.." />
    <xsl:param name="sfw_class" />

    <xsl:attribute name="data-sfw-class">
      <xsl:choose>
        <xsl:when test="$sfw_class">
          <xsl:value-of select="$sfw_class" />
        </xsl:when>
        <xsl:when test="$schema and $schema/@type">
          <xsl:value-of select="$schema/@type" />
        </xsl:when>
        <xsl:when test="$gview and $gview/@type">
          <xsl:value-of select="$gview/@type" />
        </xsl:when>
        <xsl:when test="not(../@merged) and /*/@mode-type">
          <xsl:value-of select="/*/@mode-type" />
        </xsl:when>
        <xsl:when test="../@merged and $mode-type">
          <xsl:value-of select="$mode-type" />
        </xsl:when>
        <xsl:otherwise>table</xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="schema" mode="add_sfw_class_attribute">
    <xsl:param name="sfw_class" />
    <xsl:call-template name="build_sfw_class_attribute">
      <xsl:with-param name="schema" select="." />
      <xsl:with-param name="sfw_class" select="$sfw_class" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="add_sfw_class_attribute">
    <xsl:param name="sfw_class" />
    <xsl:call-template name="build_sfw_class_attribute">
      <xsl:with-param name="schema" select="@schema" />
      <xsl:with-param name="sfw_class" select="$sfw_class" />
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
      <xsl:choose>
        <xsl:when test="local-name()='task'">
          <xsl:apply-templates select="." mode="resolve_url" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="." />
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="button/@url" mode="add_button_attribute">
    <xsl:attribute name="data-url">
      <xsl:apply-templates select="." mode="resolve_url" />
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="@*" mode="make_jump_link">
    <xsl:variable name="url">
      <xsl:apply-templates select="." mode="resolve_url" />
    </xsl:variable>

    <p>Click
    <xsl:element name="a">
      <xsl:attribute name="href"><xsl:value-of select="$url" /></xsl:attribute>
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

        <xsl:apply-templates select="@label" mode="resolve_refs" />
      </xsl:element>
    </xsl:if>
  </xsl:template>

  <!-- Recursive template to create button rows.
       Print 1. parent, 2. self, 3. self/buttons -->
  <xsl:template match="*" mode="construct_button_row">
    <xsl:param name="class" />
    <xsl:param name="host-type" select="'tr'" />
    <xsl:param name="position" />

    <xsl:if test="not(local-name()='buttons')">
      <xsl:variable name="par" select="parent::*" />
      <xsl:if test="$par">
        <xsl:apply-templates select="$par" mode="construct_button_row">
          <xsl:with-param name="class" select="$class" />
          <xsl:with-param name="host-type" select="$host-type" />
          <xsl:with-param name="position" select="$position" />
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
    
    <xsl:if test="(not($position) and not(@position)) or $position = @position">
      <xsl:variable name="buttons" select="button" />

      <xsl:if test="count($buttons)&gt;0">
        <xsl:choose>
          <xsl:when test="$host-type='tr'">
            <tr class="{$host_class}">
              <td colspan="99" style="background-color #66FF66">
                <xsl:apply-templates select="$buttons" mode="construct_button" />
              </td>
            </tr>
          </xsl:when>
          <xsl:otherwise>
            <p class="{$host_class}">
              <xsl:apply-templates select="$buttons" mode="construct_button" />
            </p>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:if>

    <xsl:if test="not(local-name()='buttons')">
      <xsl:apply-templates select="buttons" mode="construct_button_row">
        <xsl:with-param name="class" select="$class" />
        <xsl:with-param name="host-type" select="$host-type" />
        <xsl:with-param name="position" select="$position" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="seek_attribute">
    <xsl:param name="name" />
    <xsl:variable name="attr" select="@*[local-name()=$name]" />
    <xsl:choose>
      <xsl:when test="$attr"><xsl:value-of select="$attr" /></xsl:when>
      <xsl:when test="parent::*">
        <xsl:apply-templates select="parent::*" mode="seek_attribute">
          <xsl:with-param name="name" select="$name" />
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!-- Recursive template to create button lines.
       Print 1. parent, 2. self, 3. self/buttons -->
  <xsl:template match="*" mode="construct_buttons">
    <xsl:param name="class" />
    <xsl:param name="host-type" select="'tr'" />

    <xsl:if test="not(local-name()='buttons')">
      <xsl:variable name="par" select="parent::*" />
      <xsl:if test="$par[@buttons | @button]">
        <xsl:apply-templates select="$par" mode="construct_buttons">
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
              <xsl:apply-templates select="$buttons" mode="construct_button" />
            </td>
          </tr>
        </xsl:when>
        <xsl:otherwise>
          <xsl:element name="{$host-type}">
            <xsl:attribute name="class">
              <xsl:value-of select="$host_class" />
            </xsl:attribute>
            <xsl:apply-templates select="$buttons" mode="construct_button" />
          </xsl:element>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <xsl:if test="not(local-name()='buttons')">
      <xsl:apply-templates select="buttons" mode="construct_buttons">
        <xsl:with-param name="class" select="$class" />
        <xsl:with-param name="host-type" select="$host-type" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="add_tag_attribute">
    <xsl:variable name="tag" select="ancestor-or-self::*[@tag]" />
    <xsl:attribute name="get_value_from_row">yes</xsl:attribute>
    <xsl:if test="$tag and count($tag) &gt; 0">
      <xsl:attribute name="data-tag"><xsl:value-of select="$tag[1]/@tag" /></xsl:attribute>
    </xsl:if>
  </xsl:template>

  <xsl:template name="time_24_2_12">
    <xsl:param name="str" />
    <!-- could be hours, minutes, seconds.  Only first letter is significant" -->
    <xsl:param name="lsv" select="'minutes'" />

    <!-- interpret date -->

    <xsl:variable name="lsvd" select="substring($lsv,1,1)" />

    <xsl:variable name="hours" select="number(substring($str,1,2))" />
    <xsl:variable name="minutes" select="number(substring($str,4,2))" />
    <xsl:variable name="seconds" select="number(substring($str,7,2))" />
                  
    <xsl:variable name="lhours" select="$hours - 12" />

    <xsl:variable name="dhours">
      <xsl:choose>
        <xsl:when test="$hours=0">12</xsl:when>
        <xsl:when test="$hours=12">12</xsl:when>
        <xsl:when test="$lhours &lt; 0"><xsl:value-of select="$hours" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="$lhours" /></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="pval">
      <xsl:choose>
        <xsl:when test="$lhours &lt; 0">AM</xsl:when>
        <xsl:otherwise>PM</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- show conclusion -->

    <xsl:value-of select="$dhours" />

    <xsl:if test="$lsvd='m' or $lsvd='s'">
      <xsl:value-of select="concat(':', substring(($minutes+100),2))" />
    </xsl:if>

    <xsl:if test="$lsvd='s'">
      <xsl:value-of select="concat(':', substring(($seconds+100),2))" />
    </xsl:if>

    <xsl:value-of select="$pval" />

  </xsl:template>

  <!-- Three xtime convenience templates matching an attribute -->
  <xsl:template match="@*" mode="xtime">
    <xsl:call-template name="time_24_2_12">
      <xsl:with-param name="str" select="." />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="@*[contains(.,'T')]" mode="xtime">
    <xsl:call-template name="time_24_2_12">
      <xsl:with-param name="str" select="substring-after(.,'T')" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="@*[contains(.,' ')]" mode="xtime">
    <xsl:call-template name="time_24_2_12">
      <xsl:with-param name="str" select="substring-after(.,' ')" />
    </xsl:call-template>
  </xsl:template>



</xsl:stylesheet>

