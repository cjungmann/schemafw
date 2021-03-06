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

  <xsl:variable name="vars" select="/*/*[@rndx][@type='variables']" />
  <xsl:variable
      name="alt_vars"
      select="/*/*[@rndx][not(@type)][count(*[local-name()=../@row-name])=1]" />

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
      <!-- <xsl:call-template name="resolve_refs"> -->
      <!--   <xsl:with-param name="str"> -->
      <!--     <xsl:if test="local-name()='task'"> -->
      <!--       <xsl:apply-templates select="." mode="fix_srm_selfref"/> -->
      <!--     </xsl:if> -->
      <!--     <xsl:value-of select="." /> -->
      <!--   </xsl:with-param> -->
      <!-- </xsl:call-template> -->
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

    <xsl:if test="not(local-name()='buttons')">
      <xsl:variable name="par" select="parent::*" />
      <xsl:if test="$par">
        <xsl:apply-templates select="$par" mode="construct_button_row">
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
          <p class="{$host_class}">
            <xsl:apply-templates select="$buttons" mode="construct_button" />
          </p>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <xsl:if test="not(local-name()='buttons')">
      <xsl:apply-templates select="buttons" mode="construct_button_row">
        <xsl:with-param name="class" select="$class" />
        <xsl:with-param name="host-type" select="$host-type" />
      </xsl:apply-templates>
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


  <!-- <xsl:template match="*" mode="get_value_from_row"> -->
  <!--   <xsl:param name="name" /> -->
  <!--   <xsl:value-of select="current()/@*[local-name()=$name]" /> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template name="get_var_value"> -->
  <!--   <xsl:param name="name" /> -->

  <!--   <xsl:variable name="prime"> -->
  <!--     <xsl:apply-templates select="$vars" mode="get_var_value"> -->
  <!--       <xsl:with-param name="name" select="$name" /> -->
  <!--     </xsl:apply-templates> -->
  <!--   </xsl:variable> -->

  <!--   <xsl:choose> -->
  <!--     <xsl:when test="string-length($prime)"> -->
  <!--       <xsl:value-of select="$prime" /> -->
  <!--     </xsl:when> -->
  <!--     <xsl:when test="count($alt_vars) &gt; 0"> -->
  <!--       <xsl:apply-templates select="$alt_vars" mode="get_var_value"> -->
  <!--         <xsl:with-param name="name" select="$name" /> -->
  <!--       </xsl:apply-templates> -->
  <!--     </xsl:when> -->
  <!--   </xsl:choose> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template name="get_data_value"> -->
  <!--   <xsl:param name="name" /> -->
  <!--   <xsl:variable name="results" select="/*/*[@rndx and not(@type='variables')]" /> -->

  <!--   <xsl:variable name="formrs"  -->
  <!--                 select="$results[count(*[local-name()=../@row-name])=1]" /> -->
  <!--   <xsl:variable name="rows" select="$formrs/*[local-name()=../@row-name]" /> -->
  <!--   <xsl:variable name="attrs" select="$rows/@*[local-name()=$name]" /> -->

  <!--   <xsl:if test="count($attrs)"> -->
  <!--     <xsl:value-of select="$attrs[1]" /> -->
  <!--   </xsl:if> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template match="*[@rndx]" mode="get_var_value"> -->
  <!--   <xsl:param name="name" /> -->
  <!--   <xsl:variable name="row" select="*[local-name()=current()/@row-name]" /> -->
  <!--   <xsl:if test="$row"> -->
  <!--     <xsl:variable name="val" select="$row[1]/@*[local-name()=$name]" /> -->
  <!--     <xsl:if test="$val"> -->
  <!--       <xsl:value-of select="$val" /> -->
  <!--     </xsl:if> -->
  <!--   </xsl:if> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template name="add_enclose_attributes"> -->
  <!--   <xsl:param name="hints" /> -->
  <!--   <xsl:variable name="c_hint" select="substring-before($hints,'|')" /> -->
  <!--   <xsl:variable name="s_hint" select="substring($hints,1 div boolean(string-length($c_hint)=0))" /> -->
  <!--   <xsl:variable name="hint" select="concat($c_hint,$s_hint)" /> -->

  <!--   <xsl:variable name="name" select="normalize-space(substring-before($hint,'='))" /> -->
  <!--   <xsl:variable name="val" select="substring-after($hint,'=')" /> -->

  <!--   <xsl:attribute name="{$name}"> -->
  <!--     <xsl:value-of select="$val" /> -->
  <!--   </xsl:attribute> -->

  <!--   <xsl:if test="string-length($c_hint)&gt;0"> -->
  <!--     <xsl:call-template name="add_enclose_attributes"> -->
  <!--       <xsl:with-param name="hints" select="substring-after($hints,'|')" /> -->
  <!--     </xsl:call-template> -->
  <!--   </xsl:if> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template name="enclose_val"> -->
  <!--   <xsl:param name="str" /> -->
  <!--   <xsl:param name="tag" /> -->
  <!--   <xsl:param name="hints" /> -->

  <!--   <xsl:element name="{$tag}"> -->
  <!--     <xsl:if test="string-length($hints)&gt;0"> -->
  <!--       <xsl:call-template name="add_enclose_attributes"> -->
  <!--         <xsl:with-param name="hints" select="$hints" /> -->
  <!--       </xsl:call-template> -->
  <!--     </xsl:if> -->
  <!--     <xsl:value-of select="$str" /> -->
  <!--   </xsl:element> -->
  <!-- </xsl:template> -->

</xsl:stylesheet>

