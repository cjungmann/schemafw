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
  <xsl:import href="sfw_field_types.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>

  <!-- <xsl:variable name="empty-list-value" select="'- - empty list - -'" /> -->
  <xsl:variable name="empty-list-value">-- empty list --</xsl:variable>

  <xsl:template match="*[@rndx][schema][not(@merged)]['form-'=substring(/*/@mode-type,1,5)]">
    <xsl:apply-templates select="schema" mode="construct_form" />
  </xsl:template>

  <xsl:template match="*[@rndx][schema][@merged][1]['form-'=substring(@merge-type,1,5)]">
    <div>
      Matched merged result with schema and merge-type starts with form-
      (ie <xsl:value-of select="@merge-type" />)
    </div>
  </xsl:template>
  
  <xsl:template match="field" mode="construct_form_single">
    <xsl:param name="data" />
    <xsl:param name="legend" select="'legend'" />

    <form method="post"
          class="Moveable"
          data-sfw-class="form-single"
          onsubmit="return false;">
      <fieldset class="Schema">
        <legend><xsl:value-of select="$legend" /></legend>
         <xsl:apply-templates select="." mode="construct_input_row">
           <xsl:with-param name="data" select="$data" />
           <xsl:with-param name="single-field" select="true()" />
         </xsl:apply-templates>
         <p class="buttons">
           <input type="submit" value="Submit" />
           <input type="button" value="Cancel" data-type="cancel" />
         </p>
      </fieldset>
    </form>
  </xsl:template>

  <xsl:template match="field[@construct_form_single][data]">
    <xsl:apply-templates select="." mode="construct_form_single">
      <xsl:with-param name="data" select="data" />
    </xsl:apply-templates>
  </xsl:template>
    
  <xsl:template match="schema" mode="construct_form">
    <xsl:param name="result-schema" />
    <xsl:param name="method" select="'post'" />
    <xsl:param name="type" />
    <xsl:param name="primary" />
    <xsl:param name="prow" select="/.." />

    <!-- rowname will be the first matching between
         sn (schema name) row name as defined by the schema
         rn (row name) as defined by the result attribute @row-name
    -->
    <xsl:variable name="sn" select="@name" />
    <xsl:variable name="rn" select="parent::node[not($sn)]/@row-name" />
    <xsl:variable name="rowname" select="$sn|$rn" />

    <xsl:variable name="mnum" select="@merged" />

    <!--
    pd (parameter data): data row explicitly included in parameters (top priority)
    sd (sibling data): data row is sibling of schema element
    fd (form data): data row from explicitly-named form-data element
    md (merged data) row from merged result
    ld (last-restort data) the first row from the first result (rndx=1) where
       the merge status matches the merge status of the schema.

    I alternately remove and restore the last resort data row ($ld) because
    it's sometimes too permissive (making false matches) and other times
    too restrictive (a form that needs data can't find it.
    -->
    <xsl:variable name="sd" select="../*[not($prow)][local-name()=$rowname][1]" />
    <xsl:variable name="fd" select="../form-data[not($prow|$sd)]/*[1]" />
    <xsl:variable
        name="md"
        select="../*[not($prow|$sd|$fd)][@merged=$mnum][@rndx]/*[local-name()=../@row-name][1]" />
    <xsl:variable
        name="ld"
        select="/*/*[not($prow|$sd|$fd|$md)][@rndx=1]/*[local-name()=../@row-name][not(@merged) and not($mnum) or @merged=$mnum][1]" />
    <xsl:variable name="data" select="$prow|$sd|$fd|$md|$ld" />

    <xsl:variable name="sfw-class">
      <xsl:choose>
        <xsl:when test="$type"><xsl:value-of select="$type" /></xsl:when>
        <xsl:when test="@merge-type"><xsl:value-of select="@merge-type" /></xsl:when>
        <xsl:when test="@type"><xsl:value-of select="@type" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="$mode-type" /></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="render_as_page" select="$mode-type='form-page'" />

    <xsl:variable name="msg">
      <xsl:call-template name="get_var_value">
        <xsl:with-param name="name" select="'msg'" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="action">
      <xsl:apply-templates select="." mode="get_form_action" />
    </xsl:variable>
    <xsl:variable name="has_action" select="string-length($action) &gt; 0" />

    <xsl:variable name="legend">
      <xsl:apply-templates select="." mode="get_form_title" />
    </xsl:variable>

    <xsl:variable name="class">
      <xsl:choose>
        <xsl:when test="$render_as_page">Embedded</xsl:when>
        <xsl:when test="string-length($primary)">Embedded</xsl:when>
        <xsl:otherwise>Moveable</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- <xsl:if test="$render_as_page"> -->
    <!--   <h2><xsl:value-of select="$legend" /></h2> -->
    <!-- </xsl:if> -->

    <xsl:element name="form">

      <xsl:if test="$mode-type='form-import'">
        <xsl:attribute name="enctype">multipart/form-data</xsl:attribute>
      </xsl:if>

      <!-- Always include action, even if empty: -->
      <xsl:attribute name="action">
        <xsl:if test="not($action='^local')">
          <xsl:value-of select="$action" />
        </xsl:if>
      </xsl:attribute>

      <xsl:attribute name="class">
        <xsl:value-of select="$class" />
      </xsl:attribute>
      <xsl:attribute name="method">
        <xsl:value-of select="$method" />
      </xsl:attribute>
      <xsl:attribute name="data-sfw-class">
        <xsl:value-of select="$sfw-class" />
      </xsl:attribute>

      <xsl:if test="$data">
        <xsl:attribute name="data-path">
          <xsl:apply-templates select="$data" mode="gen_path" />
        </xsl:attribute>
      </xsl:if>

      <xsl:attribute name="data-schema-path">
        <xsl:apply-templates select="." mode="gen_path" />
      </xsl:attribute>

      <fieldset class="Schema">
        <xsl:if test="not($render_as_page)">
          <xsl:element name="legend">
            <xsl:attribute name="class">
              <xsl:value-of select="$class" />
            </xsl:attribute>
            <xsl:value-of select="$legend" />
          </xsl:element>
        </xsl:if>

        <xsl:apply-templates select="." mode="show_intro" />

        <xsl:apply-templates select="." mode="construct_button_row">
          <xsl:with-param name="host-type" select="'p'" />
        </xsl:apply-templates>

        <xsl:if test="$msg">
          <div class="form_msg">
            <xsl:value-of select="$msg" />
          </div>
          <hr />
        </xsl:if>

        <xsl:if test="$mode-type='form-import'">
          <p>
            <label for="upfile">Upload the file</label>
            <input type="file" name="upfile" />
          </p>
        </xsl:if>

        <xsl:apply-templates select="." mode="add_form_fields">
          <xsl:with-param name="data" select="$data" />
          <xsl:with-param name="result-schema" select="$result-schema" />
          <xsl:with-param name="view-mode" select="not($has_action)" />
        </xsl:apply-templates>

        <hr />

        <xsl:apply-templates select="." mode="construct_button_row">
          <xsl:with-param name="position" select="'bottom'" />
        </xsl:apply-templates>

        <p class="buttons">
          <xsl:if test="$has_action">
              <input type="submit" value="Submit" />
          </xsl:if>

          <xsl:variable name="btarget">
            <xsl:choose>
              <xsl:when test="$has_action">ancel</xsl:when>
              <xsl:otherwise>lose</xsl:otherwise>
            </xsl:choose>
          </xsl:variable>

          <xsl:element name="input">
            <xsl:attribute name="type">button</xsl:attribute>
            <xsl:attribute name="value">C<xsl:value-of select="$btarget" /></xsl:attribute>
            <xsl:attribute name="data-type">c<xsl:value-of select="$btarget" /></xsl:attribute>
            <xsl:if test="$class and $class!='Moveable'">
              <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
            </xsl:if>
          </xsl:element>
        </p>
      </fieldset>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema" mode="add_form_fields">
    <xsl:param name="data" select="/.." />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" select="false()" />

    <xsl:variable
        name="extra_fields"
        select="@*[substring(local-name(),1,11)='form-field-']" />

    <xsl:apply-templates
        select="$extra_fields"
        mode="construct_extra_form_field" />

      <xsl:apply-templates select="." mode="construct_form_inputs">
        <xsl:with-param name="data" select="$data" />
        <xsl:with-param name="result-schema" select="$result-schema" />
        <xsl:with-param name="view-mode" select="$view-mode" />
      </xsl:apply-templates>
  </xsl:template>

  <xsl:template name="display_error">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">
        <p class="result-msg"><xsl:value-of select="$result-row/@msg" /></p>
      </xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">
        <xsl:apply-templates select="$msg-el" mode="construct" />
      </xsl:when>
      <xsl:otherwise><p>Undefined error</p></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx][@type='variables']" mode="get_value">
    <xsl:param name="name" />
    <xsl:value-of select="*[1]/@*[local-name()=$name]" />
  </xsl:template>

  <xsl:template match="@*" mode="construct_extra_form_field">
    <xsl:variable name="name" select="substring(local-name(),12)" />
    <xsl:element name="input">
      <xsl:attribute name="type">hidden</xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="$name" /></xsl:attribute>
      <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema" mode="construct_form_inputs">
    <xsl:param name="data" />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" />

    <xsl:variable name="fields" select="field[not(@join)]" />

    <xsl:apply-templates select="$fields[@hidden]" mode="construct_hidden_input">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
    <xsl:apply-templates select="$fields[not(@hidden)]" mode="construct_form_input">
      <xsl:with-param name="data" select="$data" />
      <xsl:with-param name="result-schema" select="$result-schema" />
      <xsl:with-param name="view-mode" select="$view-mode" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field" mode="add_input_attributes">
    <xsl:variable name="list" select="@*[starts-with(local-name(),'input_')]" />

    <xsl:for-each select="$list">
      <xsl:variable name="name" select="concat('data-', substring(local-name(),7))" />
      <xsl:attribute name="{$name}">
        <xsl:apply-templates select="." mode="resolve_url" />
      </xsl:attribute>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="field" mode="construct_hidden_input">
    <xsl:param name="data" />
    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>
    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>
    <input type="hidden" name="{$name}" value="{$value}" />
  </xsl:template>

  <xsl:template match="field" mode="construct_form_input">
    <xsl:param name="data" />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" />

    <xsl:choose>
      <xsl:when test="$result-schema">
        <xsl:variable name="name" select="@name" />
        <xsl:variable
            name="result-field" select="$result-schema/field[@name=$name]" />

        <xsl:apply-templates select="." mode="construct_input_row">
          <xsl:with-param name="data" select="$data" />
          <xsl:with-param name="result-field" select="$result-field" />
          <xsl:with-param name="view-mode" select="$view-mode" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="." mode="construct_input_row">
          <xsl:with-param name="data" select="$data" />
          <xsl:with-param name="view-mode" select="$view-mode" />
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <!--
      Create a p (paragraph) element to present one row of a form.
      Each row will have a label element and some sort of input element.
  -->
  <xsl:template match="field[not(@hidden or @ignore)]" mode="construct_input_row">
    <xsl:param name="data" />
    <xsl:param name="result-field" />
    <xsl:param name="view-mode" />
    <xsl:param name="single-field" select="false()" />

    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>
    <xsl:variable name="label">
      <xsl:apply-templates select="." mode="get_label" />
    </xsl:variable>

    <xsl:variable name="class">
      <xsl:text>row</xsl:text>
      <xsl:if test="@form_class">
        <xsl:value-of select="concat(' ',@form_class)" />
      </xsl:if>
    </xsl:variable>

    <p class="form-row">
      <xsl:if test="string-length($label)">
        <label for="{$name}">
          <xsl:if test="$view-mode and @on_field_click">
            <xsl:element name="button">
              <xsl:attribute name="type">button</xsl:attribute>
              <xsl:attribute name="data-url">
                <xsl:apply-templates select="@on_field_click" mode="resolve_refs">
                  <xsl:with-param name="escape" select="1" />
                </xsl:apply-templates>
              </xsl:attribute>
              <xsl:text>Edit</xsl:text>
            </xsl:element>
          </xsl:if>
          <xsl:value-of select="$label" />
          <xsl:if test="@ebutton and not($single-field)">
            <button type="button"
                    class="ebutton_edit"
                    data-type="call"
                    data-task="SFW.process_ebutton"
                    title="Edit Field" >
            </button>
          </xsl:if>
        </label>
      </xsl:if>

      <xsl:choose>
        <xsl:when test="$single-field">
          <xsl:apply-templates select="." mode="construct_input">
            <xsl:with-param name="data" select="$data" />
            <xsl:with-param name="force-edit" select="1" />
          </xsl:apply-templates>
        </xsl:when>
        <xsl:when test="$mode-type='form-view'">
          <div class="field_content">
            <xsl:apply-templates select="." mode="display_value">
              <xsl:with-param name="data" select="$data" />
            </xsl:apply-templates>
          </div>

          <xsl:choose>
            <xsl:when test="@call">
              <button type="button" data-type="call" data-task="{@call}">edit</button>
            </xsl:when>
            <xsl:when test="@manage">
              <xsl:variable name="manage">
                <xsl:apply-templates select="@manage" mode="resolve_refs">
                  <xsl:with-param name="escape" select="1" />
                </xsl:apply-templates>
              </xsl:variable>
              <button type="button"
                      data-type="view"
                      data-url="{$manage}">Manage</button>
            </xsl:when>
          </xsl:choose>
        </xsl:when>

        <xsl:when test="$view-mode and not(@active) and not(@type='linked')">

          <div class="field_content" name="{$name}">
            <xsl:apply-templates select="." mode="display_value">
              <xsl:with-param name="data" select="$data" />
            </xsl:apply-templates>
          </div>
        </xsl:when>

        <xsl:when test="$result-field and $result-field[@enum]">
          <xsl:apply-templates select="$result-field" mode="construct_input">
            <xsl:with-param name="data" select="$data" />
          </xsl:apply-templates>
        </xsl:when>

        <xsl:otherwise>
          <xsl:apply-templates select="." mode="construct_input">
            <xsl:with-param name="data" select="$data" />
          </xsl:apply-templates>
        </xsl:otherwise>
      </xsl:choose>
    </p>
  </xsl:template>

  <xsl:template match="field" mode="display_value">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="get_value">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field[@dtd]" mode="get_list_string">
    <xsl:variable name="paren" select="substring-after(@dtd,'(')" />
    <xsl:variable name="len" select="string-length($paren)" />
    <xsl:value-of select="substring($paren,1,($len)-1)" />
  </xsl:template>

  <xsl:template match="field" mode="fill_table_cell">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="get_cell_value">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field" mode="get_s_value">
    <xsl:variable name="schema" select=".." />
    
    <xsl:variable name="sname" select="$schema/@name" />
    <xsl:variable name="srow" select="$schema/*[local-name()=$sname]" />
    <xsl:variable name="svalue">
      <xsl:if test="$srow">
        <xsl:value-of select="$srow/@*[local-name()=$sname]" />
      </xsl:if>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$svalue">
        <xsl:value-of select="$svalue" />
      </xsl:when>
      <xsl:otherwise>
        <!-- find sibling-to-schema results: -->
        <xsl:variable name="results" select="$schema/../*[@rndx]" />
        <xsl:variable name="rcount" select="count($results)" />
        <xsl:if test="$rcount&gt;0">
          <xsl:choose>
            <xsl:when test="$results[@type='data']">
              <xsl:value-of select="$results[@type='data']/@*[local-name()=$sname]" />
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="$results[1]/@*[local-name()=$sname]" />
            </xsl:otherwise>
          </xsl:choose>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>

  <xsl:template match="@*" mode="get_value_by_name">
    <xsl:choose>
      <xsl:when test="local-name()='html-value'">
        <xsl:value-of select="." />
      </xsl:when>
      <xsl:when test="local-name()='value'">
        <xsl:value-of select="." />
      </xsl:when>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="field/enum" mode="make_option">
    <xsl:param name="value" />

    <xsl:variable name="index">
      <xsl:choose>
        <xsl:when test="@index">
          <xsl:value-of select="@index" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@value" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <xsl:element name="option">
      <xsl:attribute name="value"><xsl:value-of select="$index" /></xsl:attribute>
      <xsl:if test="$index=$value">
        <xsl:attribute name="selected">selected</xsl:attribute>
      </xsl:if>
      <xsl:value-of select="@value" />
    </xsl:element>
  </xsl:template>

  <xsl:template name="construct_enum_options">
    <xsl:param name="value" />
    <xsl:param name="str" />

    <xsl:variable name="delim" select="substring($str,1,1)" />
    <xsl:variable name="tween" select="concat($delim,',',$delim)" />

    <xsl:variable name="start" select="substring($str,2)" />
    <xsl:variable name="before_tween" select="substring-before($start,$tween)" />

    <xsl:variable name="tval">
      <xsl:choose>
        <xsl:when test="$before_tween">
          <xsl:value-of select="$before_tween" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="substring($start,1,-1+string-length($start))" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="cval" select="translate($tval,$aposaka,$apos)" />

    <xsl:element name="option">
      <xsl:attribute name="value"><xsl:value-of select="$cval" /></xsl:attribute>
      <xsl:if test="$tval=$value">
        <xsl:attribute name="selected">selected</xsl:attribute>
      </xsl:if>
      <xsl:value-of select="$cval" />
    </xsl:element>

    <xsl:if test="$before_tween">
      <xsl:variable name="pos_next" select="string-length($cval)+4" />

      <xsl:call-template name="construct_enum_options">
        <xsl:with-param name="value" select="$value" />
        <xsl:with-param name="str" select="substring($str,$pos_next)" />
      </xsl:call-template>
    </xsl:if>

  </xsl:template>

  <xsl:template match="field" mode="check_readonly">
    <xsl:choose>
      <xsl:when test="@html-readonly">true</xsl:when>
      <xsl:when test="@readOnly">true</xsl:when>
      <xsl:when test="@primary-key">true</xsl:when>
      <xsl:otherwise>false</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="field" mode="construct_input">
    <xsl:param name="data" />
    <xsl:param name="force-edit" />

    <xsl:variable name="type">
      <xsl:choose>
        <xsl:when test="@html-type"><xsl:value-of select="@html-type" /></xsl:when>
        <!-- <xsl:when test="@type='DATE'">dtp_date</xsl:when> -->
        <!-- <xsl:when test="@type='DATETIME'">dtp_datetime</xsl:when> -->
        <xsl:when test="@type='DATE'">input</xsl:when>
        <xsl:when test="@type='DATETIME'">datetime</xsl:when>
        <xsl:when test="@type='BOOL'">checkbox</xsl:when>
        <xsl:when test="translate(@type,$lowers,$uppers)='PASSWORD'">password</xsl:when>
        <xsl:otherwise>text</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="class">
      <xsl:if test="@type='DATE'">dpicker</xsl:if>
    </xsl:variable>

    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:variable name="size">
      <xsl:choose>
        <xsl:when test="@html-size"><xsl:value-of select="@html-size" /></xsl:when>
        <xsl:otherwise>
            <xsl:apply-templates select="." mode="get_size" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="readonly">
      <xsl:if test="not($force-edit)">
        <xsl:apply-templates select="." mode="check_readonly" />
      </xsl:if>
    </xsl:variable>

    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:element name="input">

      <xsl:attribute name="type"><xsl:value-of select="$type" /></xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="$name" /></xsl:attribute>
      <xsl:attribute name="size"><xsl:value-of select="$size" /></xsl:attribute>

      <xsl:if test="string-length($class)&gt;0">
        <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      </xsl:if>

      <xsl:if test="$readonly='true'">
        <xsl:attribute name="readonly">readonly</xsl:attribute>
      </xsl:if>
      
      <xsl:apply-templates select="@*" mode="add_html_attribute">
        <xsl:with-param name="skip" select="' type name size readonly value '" />
      </xsl:apply-templates>

      <xsl:if test="@custom_class">
        <xsl:attribute name="data-sfw-class"><xsl:value-of select="@custom_class" /></xsl:attribute>
        <xsl:attribute name="data-sfw-control">true</xsl:attribute>
      </xsl:if>

      <xsl:apply-templates select="." mode="add_input_attributes" />

      <xsl:if test="string-length($value)&gt;0">
        <xsl:choose>
          <xsl:when test="$type='checkbox'">
            <xsl:if test="$value=1">
              <xsl:attribute name="checked">checked</xsl:attribute>
            </xsl:if>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="value">
              <xsl:value-of select="$value" />
            </xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>

    </xsl:element>

    <xsl:if test="$type='password'">
      <img class="pwtoggle"
           data-sfw-class="pwtoggle"
           data-sfw-control="true"
           src="includes/pwshow.png"
           alt="Toggle password legibility" />
    </xsl:if>

  </xsl:template>


  <xsl:template match="field[@type='TEXT']" mode="construct_input">
    <xsl:param name="data" />

    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:variable name="readonly">
      <xsl:apply-templates select="." mode="check_readonly" />
    </xsl:variable>

    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="rows">
      <xsl:choose>
        <xsl:when test="@rows"><xsl:value-of select="@rows" /></xsl:when>
        <xsl:otherwise>2</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="cols">
      <xsl:choose>
        <xsl:when test="@cols"><xsl:value-of select="@cols" /></xsl:when>
        <xsl:otherwise>30</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:element name="textarea">
      <xsl:attribute name="name"><xsl:value-of select="$name" /></xsl:attribute>
      <xsl:attribute name="rows"><xsl:value-of select="$rows" /></xsl:attribute>
      <xsl:attribute name="cols"><xsl:value-of select="$cols" /></xsl:attribute>
      <xsl:value-of select="$value" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@type='ENUM' or @enum]" mode="construct_input">
    <xsl:param name="data" />
    
    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="tstr">
      <xsl:apply-templates select="." mode="get_list_string" />
    </xsl:variable>
    <xsl:variable name="str">
      
      <xsl:call-template name="translate_apospairs">
        <xsl:with-param name="str" select="$tstr" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:element name="select">
      <xsl:attribute name="name"><xsl:value-of select="$name" /></xsl:attribute>
      <xsl:call-template name="construct_enum_options">
        <xsl:with-param name="str" select="$str" />
        <xsl:with-param name="value" select="translate($value,$apos,$aposaka)" />
      </xsl:call-template>
    </xsl:element>
    
  </xsl:template>

  <!--
      This template will be discarded when sfw_form.xsl is imported
      to a stylesheet that already includes <xsl:template match="/">.
  -->
  <xsl:template match="/">
    <html>
      <head>
        <title>Testing</title>
      </head>
      <body>
        <xsl:apply-templates select="*/schema" mode="construct_form" />
      </body>
    </html>
  </xsl:template>



</xsl:stylesheet>
