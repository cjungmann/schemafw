<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_generics.xsl" />
  <xsl:import href="sfw_utilities.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>

  <!-- <xsl:variable name="empty-list-value" select="'- - empty list - -'" /> -->
  <xsl:variable name="empty-list-value">-- empty list --</xsl:variable>

  <xsl:template match="schema" mode="make_form">
    <xsl:param name="result-schema" />
    <xsl:param name="method" select="'post'" />
    <xsl:param name="type" />
    <xsl:param name="primary" />

    <!--
        $data will be a single data element. Select with the following priority:
        1. ($fd) if current() is first gen schema, look for first row of first result
        2. ($sd) sibling of schema with proper name
        3. ($dd) 1st row of data-result, or
        4. ($rd) first child of first result (which might be a schema)
    -->
    <xsl:variable name="fd" select="../*[current()=/*/schema[1]][@rndx='1']/*[1]"/>
    <xsl:variable name="sd" select="../*[not($fd)][local-name()=current()/@name][1]"/>
    <xsl:variable name="dd" select="../form-data[not($fd|$sd)]/*[1]"/>
    <xsl:variable name="rd" select="../*[not($fd|$sd|$dd)][@rndx='1']/*[1]"/>
    <xsl:variable name="data" select="$fd|$sd|$dd|$rd"/>

    <xsl:variable name="mt-type" select="/*/@mode-type"/>

    <xsl:variable name="msg">
      <xsl:call-template name="get_var_value">
        <xsl:with-param name="name" select="'msg'" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="importing">
      <xsl:if test="/*[@mode-type='form-import']">1</xsl:if>
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
        <xsl:when test="$primary">Embedded</xsl:when>
        <xsl:otherwise>Moveable</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:element name="form">
      <xsl:if test="$importing='1'">
        <xsl:attribute name="enctype">multipart/form-data</xsl:attribute>
      </xsl:if>
      <xsl:if test="$has_action">
        <xsl:attribute name="action"><xsl:value-of select="$action" /></xsl:attribute>
      </xsl:if>

      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:attribute name="method"><xsl:value-of select="$method" /></xsl:attribute>
      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:attribute
          name="data-sfw-class"><xsl:value-of select="$type" /></xsl:attribute>
      <xsl:attribute name="data-result-type">form</xsl:attribute>
      
      <fieldset class="Schema">
        <xsl:element name="legend">
          <xsl:if test="$primary">
            <xsl:attribute name="class">Embedded</xsl:attribute>
          </xsl:if>
          <xsl:value-of select="$legend" />
        </xsl:element>

        <xsl:apply-templates select="." mode="show_buttons">
          <xsl:with-param name="host-type" select="'div'" />
        </xsl:apply-templates>

        <xsl:if test="$msg">
          <div class="form_msg">
            <xsl:value-of select="$msg" />
          </div>
          <hr />
        </xsl:if>

        <xsl:if test="$importing=1">
          <p>
            <label for="upfile">Upload the file</label>
            <input type="file" name="upfile" />
          </p>
        </xsl:if>
        
        <xsl:variable name="extra_fields"
                      select="@*[substring(local-name(),1,11)='form-field-']" />
        <xsl:apply-templates select="$extra_fields" mode="add_form_field" />

        <xsl:apply-templates select="." mode="make_form_inputs">
          <xsl:with-param name="data" select="$data" />
          <xsl:with-param name="result-schema" select="$result-schema" />
          <xsl:with-param name="view-mode" select="not($has_action)" />
        </xsl:apply-templates>
        <hr />
        <p class="buttons">
          <xsl:choose>
            <xsl:when test="$has_action">
              <input type="submit" value="Submit" />
              <input type="button" value="Cancel" class="{$class}" data-type="cancel" />
            </xsl:when>
            <xsl:otherwise>
              <input type="button" value="Close" class="{$class}" data-type="close" />
            </xsl:otherwise>
          </xsl:choose>
        </p>
      </fieldset>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema" mode="get_form_title">
    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str">
        <xsl:choose>
          <xsl:when test="@title"><xsl:value-of select="@title" /></xsl:when>
          <xsl:when test="../@title"><xsl:value-of select="../@title" /></xsl:when>
          <xsl:otherwise>Dialog</xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="schema" mode="get_form_action">
    <xsl:variable name="ta" select="@form-action" />
    <xsl:variable name="fa" select="parent::*[not($ta)]/@form-action" />
    <xsl:variable name="action" select="$ta|$fa" />
    <xsl:if test="$action">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="$action" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_name">
    <xsl:choose>
      <xsl:when test="@html-name">
        <xsl:value-of select="@html-name" />
      </xsl:when>
      <xsl:when test="@orgname">
        <xsl:value-of select="@orgname" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@name" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_label">
    <xsl:choose>
      <xsl:when test="@label">
        <xsl:apply-templates select="@label" mode="resolve_refs" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="translate(@name,'_',' ')" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx][@type='variables']" mode="get_value">
    <xsl:param name="name" />
    <xsl:value-of select="*[1]/@*[local-name()=$name]" />
  </xsl:template>

  <xsl:template match="schema/field" mode="get_cell_value">
    <xsl:param name="data" />
    <xsl:variable name="val">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="@type='BOOL'">
        <xsl:if test="$val=1">x</xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$val" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
    

  <xsl:template match="schema" mode="make_form_inputs">
    <xsl:param name="data" />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" />

    <xsl:apply-templates select="field[@hidden]" mode="make_hidden_input">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
    <xsl:apply-templates select="field[not(@hidden)]" mode="make_form_input">
      <xsl:with-param name="data" select="$data" />
      <xsl:with-param name="result-schema" select="$result-schema" />
      <xsl:with-param name="view-mode" select="$view-mode" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="schema/field" mode="make_hidden_input">
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

  <xsl:template match="field" mode="make_form_input">
    <xsl:param name="data" />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" />

    <xsl:choose>
      <xsl:when test="$result-schema">
        <xsl:variable name="name" select="@name" />
        <xsl:variable
            name="result-field" select="$result-schema/field[@name=$name]" />

        <xsl:apply-templates select="." mode="make_input_line">
          <xsl:with-param name="data" select="$data" />
          <xsl:with-param name="result-field" select="$result-field" />
          <xsl:with-param name="view-mode" select="$view-mode" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="." mode="make_input_line">
          <xsl:with-param name="data" select="$data" />
          <xsl:with-param name="view-mode" select="$view-mode" />
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>

    
  </xsl:template>

  <xsl:template match="schema/field[not(@hidden or @ignore)]" mode="make_input_line">
    <xsl:param name="data" />
    <xsl:param name="result-field" />
    <xsl:param name="view-mode" />

    <xsl:variable name="mtype" select="/*/@mode-type" />

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

    <xsl:if test="@type='block'">
      <tr><td colspan="99">
      <hr />
      </td></tr>
    </xsl:if>

    <p class="{$class}">
      
      <label for="{$name}">
        <xsl:value-of select="$label" />
      </label>

      <xsl:choose>
        <xsl:when test="$mtype='form-view'">
          <xsl:choose>
            <xsl:when test="@type='block'">
              <!-- template will enclose in div.field_content: -->
              <xsl:apply-templates select=".">
                <xsl:with-param name="data" select="$data" />
              </xsl:apply-templates>
            </xsl:when>
            <xsl:otherwise>
              <div class="field_content">
                <xsl:apply-templates select="." mode="get_value">
                  <xsl:with-param name="data" select="$data" />
                </xsl:apply-templates>
              </div>
            </xsl:otherwise>
          </xsl:choose>

          <xsl:choose>
            <xsl:when test="@call">
              <button type="button" data-type="call" data-task="{@call}">edit</button>
            </xsl:when>
            <xsl:when test="@manage">
              <xsl:variable name="manage">
                <xsl:apply-templates select="@manage" mode="resolve_refs" />
              </xsl:variable>
              <button type="button"
                      data-type="view"
                      data-url="{$manage}">Manage</button>
            </xsl:when>
          </xsl:choose>
        </xsl:when> <!-- $mtype='form-view' -->

        <xsl:when test="$view-mode">
          <div class="field_content" name="{$name}">
            <xsl:apply-templates select="." mode="get_value">
              <xsl:with-param name="data" select="$data" />
            </xsl:apply-templates>
          </div>
        </xsl:when>
        
        <xsl:when test="$result-field and $result-field[@enum]">
          <xsl:apply-templates select="$result-field" mode="make_input">
            <xsl:with-param name="data" select="$data" />
          </xsl:apply-templates>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="." mode="make_input">
            <xsl:with-param name="data" select="$data" />
          </xsl:apply-templates>
        </xsl:otherwise>
      </xsl:choose>
    </p>
  </xsl:template>


  <xsl:template match="schema/field[@dtd]" mode="get_list_string">
    <xsl:variable name="paren" select="substring-after(@dtd,'(')" />
    <xsl:variable name="len" select="string-length($paren)" />
    <xsl:value-of select="substring($paren,1,($len)-1)" />
  </xsl:template>

  <xsl:template match="schema/field" mode="fill_table_cell">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="get_cell_value">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_s_value">
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
      <xsl:when test="local-name()='ref-value'">
        <xsl:apply-templates select="$vars" mode="get_value">
          <xsl:with-param name="name" select="." />
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_value">
    <xsl:param name="data" />
    <xsl:variable name="name" select="@name" />

    <xsl:choose>
      <xsl:when test="@html-value">
        <xsl:value-of select="@html-value" />
      </xsl:when>
      <xsl:when test="@value">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@value" />
        </xsl:call-template>
        <!-- <xsl:value-of select="@value" /> -->
      </xsl:when>
      <xsl:when test="@ref-value">
        <xsl:apply-templates select="$vars" mode="get_value">
          <xsl:with-param name="name" select="@ref-value" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="@map-value">
        <xsl:value-of select="$data/@*[local-name()=current()/@map-value]" />
      </xsl:when>
      <xsl:when test="$data and (not(@nodetype) or @nodetype='attribute')">
        <xsl:variable name="v" select="$data/@*[name()=$name]" />
        <xsl:if test="$v">
          <xsl:value-of select="$v" />
        </xsl:if>
      </xsl:when>
      <xsl:when test="$data and @nodetype='child'">
        <xsl:variable name="v" select="$data/*[local-name()=$name]" />
        <xsl:if test="$v">
          <xsl:value-of select="$v" />
        </xsl:if>
      </xsl:when>
      
      <xsl:otherwise>
        <xsl:value-of select="$data" />
      </xsl:otherwise>
      
      <!-- <xsl:otherwise> -->
      <!--   <xsl:apply-templates select="." mode="get_s_value" /> -->
      <!-- </xsl:otherwise> -->
      
    </xsl:choose>

  </xsl:template>

  <xsl:template match="schema/field" mode="get_size">
    <xsl:choose>
      <xsl:when test="@length &lt; 25">
        <xsl:value-of select="@length" />
      </xsl:when>
      <xsl:otherwise>25</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field/enum" mode="make_option">
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

  <xsl:template name="make_enum_options">
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

      <xsl:call-template name="make_enum_options">
        <xsl:with-param name="value" select="$value" />
        <xsl:with-param name="str" select="substring($str,$pos_next)" />
      </xsl:call-template>
    </xsl:if>

  </xsl:template>

  <xsl:template match="schema/field[@type='ENUM' or @enum]" mode="make_input" priority="10">
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
      <xsl:call-template name="make_enum_options">
        <xsl:with-param name="str" select="$str" />
        <xsl:with-param name="value" select="translate($value,$apos,$aposaka)" />
      </xsl:call-template>
    </xsl:element>
    
  </xsl:template>

  <xsl:template match="schema/field" mode="make_input">
    <xsl:param name="data" />

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
      <xsl:choose>
        <xsl:when test="@html-readonly">true</xsl:when>
        <xsl:when test="@readOnly">true</xsl:when>
      </xsl:choose>
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
        <xsl:attribute name="disabled">disabled</xsl:attribute>
        <xsl:attribute name="readonly">readonly</xsl:attribute>
      </xsl:if>
      
      <xsl:attribute name="fake">Fake</xsl:attribute>
      
      <xsl:apply-templates select="@*" mode="add_html_attributes">
        <xsl:with-param name="skip" select="' type name size readonly value '" />
      </xsl:apply-templates>

      <xsl:attribute name="bogus">Bogus</xsl:attribute>
      
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
  </xsl:template>

  <xsl:template match="schema/field[@lookup]" mode="make_input">
    <select name="{@name}" data-url="{@lookup}">
      <option value="1">One</option>
      <option value="2">Two</option>
    </select>
  </xsl:template>

  <xsl:template match="schema/field[@type='block']">
    <xsl:param name="data" />
    
    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:element name="div">
      <xsl:attribute name="class">field_content</xsl:attribute>
      <xsl:apply-templates
          select="@update|@result"
          mode="add_resolved_data_attribute" />

      <xsl:choose>
        <xsl:when test="$data">
          <xsl:apply-templates select="." mode="make_list">
            <xsl:with-param name="str" select="$data/@*[local-name()=$name]" />
          </xsl:apply-templates>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="." mode="make_list" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:element>
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

  <xsl:template name="make_list">
    <xsl:param name="str" />
    <xsl:param name="rdelim" />
    <xsl:variable name="before" select="substring-before($str, $rdelim)" />
    
    <xsl:variable name="val">
      <xsl:choose>
        <xsl:when test="$before"><xsl:value-of select="$before" /></xsl:when>
        <xsl:otherwise><xsl:value-of select="$str" /></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:call-template name="make_line_of_list">
      <xsl:with-param name="str" select="$val" />
    </xsl:call-template>

    <xsl:if test="$before">
      <xsl:call-template name="make_list">
        <xsl:with-param name="str" select="substring-after($str,$rdelim)" />
        <xsl:with-param name="rdelim" select="$rdelim" />
      </xsl:call-template>
    </xsl:if>
    
  </xsl:template>

  <xsl:template match="schema/field[@type='block' and not(@result)]" mode="make_list">
    <xsl:param name="str" />
    <xsl:variable name="rdelim">
      <xsl:choose>
        <xsl:when test="@rdelim"><xsl:value-of select="@rdelim" /></xsl:when>
        <xsl:otherwise>;</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <div>
      <xsl:choose>
        <xsl:when test="string-length($str)&gt;1">
          <xsl:call-template name="make_list">
            <xsl:with-param name="str" select="$str" />
            <xsl:with-param name="rdelim" select="$rdelim" />
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <div><xsl:value-of select="$empty-list-value" /></div>
        </xsl:otherwise>
      </xsl:choose>
    </div>
  </xsl:template>

  <xsl:template name="make_line_of_list">
    <xsl:param name="str" />
    <div>
      <xsl:value-of select="$str" />
    </div>
  </xsl:template>


  <xsl:template match="schema/field[@type='block' and @result]" mode="make_list">
    <xsl:variable
        name="result"
        select="/*/*[@rndx][local-name()=current()/@result]" />
    
    <xsl:choose>
      <xsl:when test="count($result/*)">
        <xsl:call-template name="make_line_of_list">
          <xsl:with-param name="str" select="$result/@*[1]" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <div>
          <xsl:value-of select="$empty-list-value" />
        </div>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field[@type='block']">
    <xsl:param name="data" />
    
    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:element name="div">
      <xsl:attribute name="class">field_content</xsl:attribute>
      <xsl:apply-templates
          select="@update|@result"
          mode="add_resolved_data_attribute" />

      <xsl:choose>
        <xsl:when test="$data">
          <xsl:apply-templates select="." mode="make_list">
            <xsl:with-param name="str" select="$data/@*[local-name()=$name]" />
          </xsl:apply-templates>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="." mode="make_list" />
        </xsl:otherwise>
      </xsl:choose>
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
        <xsl:apply-templates select="*/schema" mode="make_form" />
      </body>
    </html>
  </xsl:template>



</xsl:stylesheet>
