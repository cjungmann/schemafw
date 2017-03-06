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

  <xsl:variable name="table_lines" select="/" />

  <xsl:variable name="nl">
    <xsl:text>
</xsl:text>
  </xsl:variable>

  <xsl:variable name="vars_obj" select="'sfwvars'" />
  <xsl:variable name="vars" select="/*/*[@rndx][@type='variables']" />
  <xsl:variable name="empty-list-value" select="'-- empty list --'" />

  <xsl:variable name="apos"><xsl:text>&apos;</xsl:text></xsl:variable>
  <xsl:variable name="aposcomapos">&apos;,&apos;</xsl:variable>
  <xsl:variable name="apospair">&apos;&apos;</xsl:variable>
  <xsl:variable name="aposaka">&#127;</xsl:variable>

  <xsl:variable name="result-row"
                select="/*[@mode-type='form-result']/*[@rndx=1]/*[@error]" />

  <xsl:variable name="docel_msg" select="/message" />
  <xsl:variable name="child_msg" select="/*[not($docel_msg)]/message" />
  <xsl:variable name="msg-el" select="$docel_msg | $child_msg" />

  <xsl:variable name="jslist_sfw_brief">sfw</xsl:variable>
  <xsl:variable name="jslist_sfw_minified">sfw.min</xsl:variable>
  <xsl:variable name="jslist_sfw_debug">sfw_0 sfw_dom sfw_table sfw_form sfw_form_view sfw_calendar sfw_debug sfw_onload</xsl:variable>
  <xsl:variable name="jslist_sfw" select="$jslist_sfw_debug" /> 
  <xsl:variable name="jslist_utils">classes dpicker Events Dialog Moveable XML</xsl:variable>

  <xsl:variable name="lowers">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="uppers">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

  <xsl:variable name="err_condition">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">1</xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>


  <xsl:template name="display_error">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">
        <p class="result-msg"><xsl:value-of select="$result-row/@msg" /></p>
      </xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">
        <xsl:apply-templates select="$msg-el" />
      </xsl:when>
      <xsl:otherwise><p>Undefined error</p></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="tag_class">
    <xsl:param name="type" />
    <xsl:attribute name="data-sfw-class">
      <xsl:value-of select="$type" />
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="/*[@mode-type='form-edit']" >
    <xsl:param name="root" />

    <xsl:variable name="form-type">
      <xsl:choose>
        <xsl:when test="@form-type"><xsl:value-of select="@form-type" /></xsl:when>
        <xsl:when test="$root">form</xsl:when>
        <xsl:otherwise>dialog</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="schema" select="schema | *[@rndx]/schema" />

    <xsl:choose>
      <xsl:when test="$schema">
        <xsl:apply-templates select="$schema" mode="make_form">
          <xsl:with-param name="type" select="$form-type" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>Unable to find the schema for constructing the form.</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="/*[@mode-type='form-new' or @mode-type='form-import']" >
    <xsl:param name="root" />

    <xsl:variable name="form-type">
      <xsl:choose>
        <xsl:when test="@form-type"><xsl:value-of select="@form-type" /></xsl:when>
        <xsl:when test="$root">form</xsl:when>
        <xsl:otherwise>dialog</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="schema">
        <xsl:apply-templates select="schema" mode="make_form">
          <xsl:with-param name="type" select="$form-type" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>Unable to construct a form without a schema</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="/*[@mode-type='form-view']">
    <xsl:param name="root" />

    <xsl:variable name="form-type">
      <xsl:choose>
        <xsl:when test="@form-type"><xsl:value-of select="@form-type" /></xsl:when>
        <xsl:when test="$root">form</xsl:when>
        <xsl:otherwise>dialog</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="fschema" select="schema" />
    <xsl:variable name="rschema" select="*[not($fschema)][@rndx=1]/schema" />
    <xsl:variable name="schema" select="$fschema | $rschema" />

    <xsl:choose>
      <xsl:when test="$schema">
        <xsl:apply-templates select="$schema" mode="make_form">
          <xsl:with-param name="type" select="$form-type" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>Unable to construct a form without a schema</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="/*[@mode-type='table']">
    <xsl:variable name="rt" select="*[@rndx][@type='table']" />
    <xsl:variable name="r1" select="*[not($rt)][@rndx=1]" />
    <xsl:variable name="result" select="$rt | $r1" />

    <xsl:choose>
      <xsl:when test="$result and $result/schema">
        <xsl:apply-templates select="$result" mode="make_table" />
      </xsl:when>
      <xsl:otherwise>
        <div>Could not find an appropriate result.</div>
      </xsl:otherwise>
    </xsl:choose>
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

    <!-- <xsl:variable name="before" select="substring-before($str, '{@')" /> -->
    
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

  <xsl:template name="add_js">
    <xsl:param name="list" />

    <xsl:variable name="before" select="substring-before($list, ' ')" />

    <xsl:variable name="file">
      <xsl:choose>
        <xsl:when test="string-length($before)">
          <xsl:value-of select="$before" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$list" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="string-length($file)&gt;0">
      <xsl:variable name="path" select="concat('includes/', $file, '.js')" />
      <script type="text/javascript" src="{$path}"></script>
    </xsl:if>

    <xsl:if test="string-length($before)">
      <xsl:value-of select="$nl" />
      <xsl:call-template name="add_js">
        <xsl:with-param name="list" select="substring-after($list,' ')" />
      </xsl:call-template>
    </xsl:if>
    
  </xsl:template>

  <xsl:template name="js_includes">
    <xsl:call-template name="add_js">
      <xsl:with-param name="list" select="$jslist_sfw" />
    </xsl:call-template>
    <xsl:call-template name="add_js">
      <xsl:with-param name="list" select="$jslist_utils" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="css_includes">
    <link rel="stylesheet" type="text/css" href="includes/schemafw.css" />
    <xsl:value-of select="$nl" />
    <link rel="stylesheet" type="text/css" href="includes/dpicker.css" />
    <xsl:value-of select="$nl" />
  </xsl:template>

  <xsl:template match="@meta-jump" mode="make_link">
    <p>Click <a href="{@meta-jump}">here</a> if you're not taken there.</p>
  </xsl:template>

  <!--Elminate extra output for unmatched elements and attributes -->
  <xsl:template match="*" mode="add_to_head"></xsl:template>
  <xsl:template match="@*" mode="add_to_head"></xsl:template>

  <xsl:template match="@meta-jump" mode="add_to_head">
    <!-- <xsl:if test="not($result-row) or $result-row/@error=0"> -->
    <xsl:if test="$err_condition='0'">
      <xsl:variable name="content" select="concat('0; url=', .)" />
      <meta http-equiv="refresh" content="{$content}" />
      <xsl:value-of select="$nl" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="meta-jump" mode="add_to_head">
    <xsl:variable name="wait">
      <xsl:choose>
        <xsl:when test="@wait"><xsl:value-of select="@wait" /></xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <xsl:variable name="url">
      <xsl:choose>
        <xsl:when test="@url">
          <xsl:apply-templates select="@url" mode="resolve_refs" />
        </xsl:when>
        <xsl:otherwise>/</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="content">
      <xsl:value-of select="concat($wait,'; ', 'url=', $url)" />
    </xsl:variable>

    <meta http-equiv="refresh" content="{$content}" />
    <xsl:value-of select="$nl" />
  </xsl:template>

  <xsl:template match="@mode-type" mode="add_to_head">
    <meta name="sfw-mode-type" content="{.}" />
    <xsl:value-of select="$nl" />
  </xsl:template>

  <xsl:template match="script" mode="add_to_head">
    <script type="text/javascript" src="{@src}"></script>
    <xsl:value-of select="$nl" />
  </xsl:template>

  <xsl:template match="@script" mode="add_to_head">
    <script type="text/javascript" src="{.}"></script>
    <xsl:value-of select="$nl" />
  </xsl:template>

  <xsl:template match="@*" mode="add_attribute_variable">
    <xsl:variable name="n" select="translate(local-name(),'-', '_')" />
    <xsl:value-of select="concat($vars_obj, '.' , $n, ' = &quot;', ., '&quot;;', $nl)" />
  </xsl:template>

  <xsl:template match="*[@rndx][@type='variables']" mode="add_to_head">
    <xsl:if test="count(*)">
      <script type="text/javascript">
        <xsl:value-of select="$nl" />
        <xsl:apply-templates select="*/@*" mode="add_attribute_variable" />
      </script>
      <xsl:value-of select="$nl" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="/*" mode="add_document_variables">
    <script type="text/javascript">
      <xsl:value-of select="$nl" />
      <xsl:apply-templates select="@*" mode="add_attribute_variable" />
    </script>
    <xsl:value-of select="$nl" />
  </xsl:template>



  <xsl:template match="/*" mode="fill_head">
    <xsl:choose>
      <xsl:when test="$err_condition&gt;0">
        <!-- do nothing -->
      </xsl:when>
      <!-- <xsl:when test="$result-row and $result-row/@error=0 and @meta-jump"> -->
      <xsl:when test="@meta-jump">
        <script type="text/javascript">
          <xsl:text>location.replace(&quot;</xsl:text>
          <xsl:value-of select="@meta-jump" />
          <xsl:text>&quot;);</xsl:text>
        </script>
      </xsl:when>

      <xsl:otherwise>
        <xsl:value-of select="$nl" />
        <xsl:call-template name="css_includes" />
        <xsl:call-template name="js_includes" />
        <xsl:if test="count($vars)">
          <script type="text/javascript">var <xsl:value-of select="$vars_obj" />={};</script>
          <xsl:value-of select="$nl" />
          <xsl:apply-templates select="." mode="add_document_variables" />
        </xsl:if>
        <xsl:apply-templates select="@*" mode="add_to_head" />
      </xsl:otherwise>

    </xsl:choose>
  </xsl:template>

  <xsl:template match="navigation/target" mode="header">
    <a href="{@url}"><xsl:value-of select="@label" /></a>
  </xsl:template>

  <xsl:template match="/*/navigation" mode="header">
    <nav>
      <xsl:apply-templates select="target" mode="header" />
    </nav>
  </xsl:template>

  <xsl:template name="write_title">
    <xsl:param name="title" />

    <xsl:variable name="resolved">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="$title" />
      </xsl:call-template>
    </xsl:variable>

    <h2 class="sfw_title"><xsl:value-of select="$resolved" /></h2>
  </xsl:template>

  <xsl:template match="*[@rndx and @title]" mode="add_title">
    <xsl:call-template name="write_title">
      <xsl:with-param name="title" select="@title" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="*[@rndx and schema/@title]" mode="add_title">
    <xsl:call-template name="write_title">
      <xsl:with-param name="title" select="schema/@title" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="/*" mode="branch_standard_modes">
    <div>Use mode=&quot;show_document_content&quot; instead of &quot;branch_standard_modes&quot;</div>
  </xsl:template>

  <xsl:template match="/*" mode="make_schemafw_meta">
    <xsl:element name="div">
      <xsl:attribute name="id">schemafw-meta</xsl:attribute>
      <xsl:attribute name="style">display:none</xsl:attribute>

      <xsl:if test="@method='POST'">
        <xsl:attribute name="data-post">true</xsl:attribute>
      </xsl:if>

      <xsl:attribute name="data-modeType">
        <xsl:value-of select="@mode-type" />
      </xsl:attribute>

      <xsl:if test="@meta-jump">
        <xsl:attribute name="data-jump">
        <xsl:value-of select="@meta-jump" />
        </xsl:attribute>
      </xsl:if>

      <xsl:if test="local-name()='message'">
        <xsl:element name="span">
          <xsl:attribute name="class">message</xsl:attribute>
          <xsl:if test="@detail">
            <xsl:attribute name="data-detail">
              <xsl:value-of select="@detail" />
            </xsl:attribute>
          </xsl:if>
          <xsl:value-of select="@message" />
        </xsl:element>
      </xsl:if>
    </xsl:element>
  </xsl:template>

  <xsl:template match="/*" mode="show_document_content">
    <xsl:variable name="formtype" select="substring-after(/*/@mode-type,'form-')" />
    
    <xsl:apply-templates select="." mode="make_schemafw_meta" />

    <xsl:if test="$err_condition&gt;0">
      <xsl:call-template name="display_error" />
      <xsl:if test="@meta-jump">
        <xsl:apply-templates select="@meta-jump" mode="make_link" />
      </xsl:if>
    </xsl:if>

    <!-- <xsl:if test="not($result-row) or not($result-row/@error=0)"> -->
    <xsl:if test="$err_condition=0 and not(@meta-jump)">
      <xsl:if test="$result-row and $result-row/@msg">
        <p class="result-msg"><xsl:value-of select="$result-row/@msg" /></p>
      </xsl:if>
      
      <xsl:choose>
        <xsl:when test="message/@type='error'">
          <xsl:apply-templates select="message" />
        </xsl:when>
        <xsl:when test="$formtype and contains('edit view new', $formtype)">
          <xsl:apply-templates select="/*">
            <xsl:with-param name="root" select="1" />
          </xsl:apply-templates>
        </xsl:when>
        <xsl:when test="schema">
          <xsl:apply-templates select="schema" mode="make_form">
            <xsl:with-param name="type" select="'form'" />
          </xsl:apply-templates>
        </xsl:when>
        <xsl:otherwise>
          <xsl:variable name="mt" select="@mode-type" />
          <xsl:variable name="mtr" select="*[@rndx and local-name()=$mt]" />
          <xsl:variable name="result" select="$mtr | *[@rndx=1 and not($mtr)]" />
          <xsl:choose>
            <xsl:when test="$result">
              <xsl:apply-templates select="$result" />
            </xsl:when>
            <xsl:otherwise>
              <div>Don't know what to do.</div>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
        <!-- <xsl:when test="@mode-type and *[@rndx and local-name()=current()/@mode-type]"> -->
        <!--   <xsl:apply-templates select="*[@rndx and local-name()=current()/@mode-type]" /> -->
        <!-- </xsl:when> -->
        <!-- <xsl:when test="*[@rndx=1]"> -->
        <!--   <xsl:apply-templates select="*[@rndx=1]" /> -->
        <!-- </xsl:when> -->
        <!-- <xsl:otherwise> -->
        <!--   <div>Don't know what to do.</div> -->
        <!-- </xsl:otherwise> -->
      </xsl:choose>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*[@rndx and not(@type='variables' or @type='info')]">
    <xsl:variable name="row_name">
      <xsl:choose>
        <xsl:when test="schema">
          <xsl:value-of select="schema/@name" />
        </xsl:when>
        <xsl:when test="@row-name">
          <xsl:value-of select="@row-name" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="@form">
        <xsl:variable name="result" select="*[@rndx][local-name()=current()/@form]" />
        <xsl:choose>
          <xsl:when test="not($result)">
            <div>In form mode, unable to find form result &quot;<xsl:value-of select="@form" />&quot;</div>
          </xsl:when>
          <xsl:when test="not($result/schema)">
            <div>In form mode, unable to find the schema in form result &quot;<xsl:value-of select="@form" />&quot;</div>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="$result/schema" mode="make_form" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:when test="schema">
        <xsl:apply-templates select="." mode="make_table" />
      </xsl:when>

      <xsl:otherwise>
        <div>Unable to find schema for building a form.</div>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema" mode="make_form">
    <xsl:param name="result-schema" />
    <xsl:param name="method" select="'post'" />
    <xsl:param name="type" select="'dialog'" />

    <!-- Get first of 1. if current() is first gen schema, look for first row of first result
                      2. sibling of schema with proper name
                      3. 1st row of data-result, or
                      4. first child of first result (which might be a schema) -->
    <xsl:variable name="fdata" select="../*[current()=/*/schema[1]][@rndx='1']/*[1]" />
    <xsl:variable name="sdata" select="../*[not($fdata)][local-name()=current()/@name][1]" />
    <xsl:variable name="ddata" select="../form-data[not($fdata|$sdata)]/*[1]" />
    <xsl:variable name="rdata" select="../*[not($fdata|$sdata|$ddata)][@rndx='1']/*[1]" />
    <!-- since each _data variable is set only if previous ones not set, union gives only one result -->
    <xsl:variable name="data" select="$fdata|$sdata|$ddata|$rdata" />

    <xsl:variable name="msg">
      <xsl:call-template name="get_var_value">
        <xsl:with-param name="name" select="'msg'" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="importing">
      <xsl:if test="/*[@mode-type='form-import']">1</xsl:if>
    </xsl:variable>

    <xsl:variable name="action">
      <xsl:choose>
        <xsl:when test="@form-action">
          <xsl:apply-templates select="@form-action" mode="resolve_refs" />
        </xsl:when>
        <xsl:when test="parent::node()[@form-action]">
          <xsl:apply-templates select="parent::node()/@form-action"
                               mode="resolve_refs" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="has_action" select="string-length($action) &gt; 0" />

    <xsl:variable name="legend">
      <xsl:choose>
        <xsl:when test="@title">
          <xsl:apply-templates select="@title" mode="resolve_refs" />
        </xsl:when>
        <xsl:when test="../@title">
          <xsl:apply-templates select="../@title" mode="resolve_refs" />
        </xsl:when>
        <xsl:otherwise>Dialog</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="class">
      <xsl:choose>
        <xsl:when test="$type='dialog'">Moveable</xsl:when>
        <xsl:otherwise>Embedded</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:element name="form">
      <xsl:call-template name="tag_class">
        <xsl:with-param name="type" select="$class" />
      </xsl:call-template>
      <xsl:attribute name="method"><xsl:value-of select="$method" /></xsl:attribute>
      <xsl:if test="$importing='1'">
        <xsl:attribute name="enctype">multipart/form-data</xsl:attribute>
      </xsl:if>
      <xsl:if test="$has_action">
        <xsl:attribute name="action"><xsl:value-of select="$action" /></xsl:attribute>
      </xsl:if>
      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:attribute name="data-result-type">form</xsl:attribute>

      <fieldset class="Schema">
        <xsl:element name="legend">
          <xsl:if test="$type='dialog'">
            <xsl:attribute name="class">Moveable</xsl:attribute>
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

  <xsl:template match="schema" mode="show_intro">
    <xsl:param name="class" />
    <xsl:param name="host-type" select="'tr'" />

    <xsl:variable name="intro">
      <xsl:choose>
        <xsl:when test="intro"><xsl:value-of select="intro" /></xsl:when>
        <xsl:when test="/*/@intro"><xsl:value-of select="/*/@intro" /></xsl:when>
      </xsl:choose>
    </xsl:variable>

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

  <xsl:template match="button/@task" priority="5" mode="data_attribute">
    <xsl:attribute name="data-task">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="." />
      </xsl:call-template>
      <xsl:apply-templates select="../param" mode="add_task_param" />
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="button/@*" mode="data_attribute">
    <xsl:param name="no-prefix" />

    <xsl:variable name="add-prefix">
      <xsl:choose>
        <xsl:when test="$no-prefix">
          <xsl:variable name="tname" select="concat(' ', local-name(), ' ')" />
          <xsl:choose>
            <xsl:when test="$tname and contains($no-prefix, $tname)">0</xsl:when>
            <xsl:otherwise>1</xsl:otherwise>
          </xsl:choose>
        </xsl:when>
        <xsl:otherwise>1</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="name">
      <xsl:choose>
        <xsl:when test="$add-prefix='1'">
          <xsl:value-of select="concat('data-', local-name())" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="local-name()" />
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

  <xsl:template match="button/param" mode="add_task_param">
    <xsl:variable name="val">
      <xsl:apply-templates select="@*[not(local-name()='name')]"
                           mode="get_value_by_name" />
    </xsl:variable>
    
    <xsl:if test="$val">
      <xsl:choose>
        <xsl:when test="@name">
          <xsl:value-of select="concat(' ', @name, '=', $val)" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat('=', $val)" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:template>

  <xsl:template match="button" mode="show">
    <xsl:element name="button">
      <xsl:attribute name="type">button</xsl:attribute>
      
      <xsl:apply-templates select="@*[not(local-name()='label')]"
                           mode="data_attribute">
        <xsl:with-param name="no-prefix" select="' name value disabled '" />
      </xsl:apply-templates>
      
      <xsl:value-of select="@label" />
    </xsl:element>
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


  <xsl:template match="@*" mode="add_form_field">
    <xsl:variable name="name" select="substring(local-name(),12)" />
    <xsl:element name="input">
      <xsl:attribute name="type">hidden</xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="$name" /></xsl:attribute>
      <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field" mode="make_form_input">
    <xsl:param name="data" />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" />

    <xsl:choose>
      <xsl:when test="$result-schema">
        <xsl:variable name="name" select="@name" />
        <xsl:variable name="result-field" select="$result-schema/field[@name=$name]" />

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

  <xsl:template match="schema" mode="make_form_inputs">
    <xsl:param name="data" />
    <xsl:param name="result-schema" />
    <xsl:param name="view-mode" />
    
    <xsl:apply-templates select="field" mode="make_hidden_input">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
    <xsl:apply-templates select="field" mode="make_form_input">
      <xsl:with-param name="data" select="$data" />
      <xsl:with-param name="result-schema" select="$result-schema" />
      <xsl:with-param name="view-mode" select="$view-mode" />
    </xsl:apply-templates>
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

  <xsl:template match="schema/field[@dtd]" mode="get_list_string">
    <xsl:variable name="paren" select="substring-after(@dtd,'(')" />
    <xsl:variable name="len" select="string-length($paren)" />
    <xsl:value-of select="substring($paren,1,($len)-1)" />
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

  <xsl:template match="schema/field" mode="fill_table_cell">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="get_cell_value">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="schema/field" mode="fill_group_cell">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="get_value">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="result[@type='variables']" mode="get_value">
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

  <xsl:template match="schema/group" mode="get_value">
    <xsl:param name="data" />
    <xsl:param name="field_name" />
    
    <xsl:variable name="field" select="../field[@name=$field_name]" />
    <xsl:if test="count($field)&gt;0">
      <xsl:apply-templates select="$field" mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_size">
    <xsl:choose>
      <xsl:when test="@length &lt; 25">
        <xsl:value-of select="@length" />
      </xsl:when>
      <xsl:otherwise>25</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field[@hidden]" mode="make_hidden_input">
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

  <xsl:template match="schema/field[@select_from_group]" mode="make_input">
    <xsl:param name="data" />

    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:element name="input">
      <xsl:attribute name="type">text</xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
      <xsl:attribute name="value"><xsl:value-of select="$value" /></xsl:attribute>
    </xsl:element>
      
    <xsl:element name="select">
      <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
    </xsl:element>

  </xsl:template>

  <xsl:template name="translate_apospairs">
    <xsl:param name="str" />
    
    <xsl:variable name="left" select="substring-before($str,$apospair)" />
    <xsl:variable name="lenleft" select="string-length($left)" />

    <xsl:choose>
      <xsl:when test="$lenleft=0">
        <xsl:value-of select="$str" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="right" select="substring-after($str,$apospair)" />
        <xsl:variable name="last_of_left" select="substring($left,$lenleft,1)" />
        <xsl:variable name="first_of_right" select="substring($right,1,1)" />
        
        <xsl:value-of select="$left" />
        
        <!-- see what comes just before and just after the apostrophe pair -->
        <xsl:choose>
          <!-- pathological case ',''', keep first apos after comma, convert next two -->
          <xsl:when test="$last_of_left=',' and $first_of_right=$apos">
            <xsl:value-of select="concat($apos,$aposaka)" />
            <xsl:call-template name="translate_apospairs">
              <xsl:with-param name="str" select="substring($right,2)" />
            </xsl:call-template>
          </xsl:when>
          <!-- convert '' to single placeholder -->
          <xsl:otherwise>
            <xsl:value-of select="$aposaka" />
            <xsl:call-template name="translate_apospairs">
              <xsl:with-param name="str" select="$right" />
            </xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
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
      
      <xsl:apply-templates select="@*" mode="add_html_attributes">
        <xsl:with-param name="skip" select="' type name size readonly value '" />
      </xsl:apply-templates>
      
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

  <xsl:template match="schema/field[@group]" mode="make_input">
    <xsl:variable name="group" select="../group[@name=current()/@group]" />

    <xsl:variable name="class">
      <xsl:text>TableEditInput</xsl:text>
      <xsl:if test="@attrib-class">
        <xsl:value-of select="concat(' ',@attrib-class)" />
      </xsl:if>
    </xsl:variable>

    <xsl:element name="input">
      <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:attribute name="data_result_name">
        <xsl:value-of select="$group/@name" />
      </xsl:attribute>

      <xsl:apply-templates select="@*" mode="add_html_attributes" />
    </xsl:element>
    
  </xsl:template>

  <xsl:template match="schema/field[@lookup]" mode="make_input">
    <select name="{@name}" data-url="{@lookup}">
      <option value="1">One</option>
      <option value="2">Two</option>
    </select>
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

  <xsl:template match="schema/field[not(@hidden or @ignore) and @group]" mode="make_input_line">
    <xsl:param name="data" />
    <xsl:param name="result-field" />
    <xsl:param name="view-mode" />

    <xsl:variable name="group" select="../group[@name=current()/@group]" />
    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    
    <p>
      <xsl:apply-templates select="." mode="make_input" />
      
      <xsl:apply-templates select="$group" mode="make_result_table">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </p>
    
  </xsl:template>

  <xsl:template match="*" mode="get_path">
    <xsl:if test="parent::*">
      <xsl:apply-templates select="parent::*" mode="get_path" />
    </xsl:if>
    <xsl:value-of select="concat('/', name())" />
    <xsl:if test="name()='result'">
      <xsl:value-of select="concat('[@rndx=', @rndx, ']')" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema" mode="get_id_field_name">
    <xsl:variable name="lid" select="./field[@line_id]" />
    <xsl:variable name="pid" select="./field[@primary-key]" />
    <xsl:choose>
      <xsl:when test="$lid">
        <xsl:value-of select="$lid/@name" />
      </xsl:when>
      <xsl:when test="$pid">
        <xsl:value-of select="$pid/@name" />
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema" mode="schema_make_table_body">
    <xsl:param name="lines" />
    <xsl:param name="group" />

    <xsl:variable name="field_gids">
      <xsl:choose>
        <xsl:when test="$group">
          <xsl:apply-templates select="$group" mode="get_field_gids" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="." mode="get_field_gids" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="id_field">
      <xsl:apply-templates select="." mode="get_id_field_name" />
    </xsl:variable>
    
    <xsl:variable name="field" select="field[@sorting]" />

    <xsl:choose>
      <xsl:when test="$field">
        <xsl:variable name="aname" select="$field/@name" />
        <xsl:variable name="dir">
          <xsl:choose>
            <xsl:when test="$field/@descending">descending</xsl:when>
            <xsl:otherwise>ascending</xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <xsl:variable name="data-type">
          <xsl:choose>
            <xsl:when test="$field/@sort"><xsl:value-of select="$field/@sort" /></xsl:when>
            <xsl:otherwise>text</xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <xsl:apply-templates select="$lines" mode="make_table_line">
          <xsl:with-param name="line_id" select="$id_field" />
          <xsl:sort select="count(@*[local-name()=$aname])"
                    data-type="number"
                    order="descending" />
          <xsl:sort select="@*[local-name()=$aname]"
                    data-type="{$data-type}"
                    order="{$dir}" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="$lines" mode="make_table_line">
          <xsl:with-param name="line_id" select="$id_field" />
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="make_table_body">
    <xsl:param name="group" />
    <xsl:variable name="elname" select="schema/@name" />

    <xsl:variable
        name="lines"
        select="*[$table_lines=/][local-name()=$elname] | $table_lines[not($table_lines=/)]"
        />
    
    <!-- <xsl:variable name="lines" select="*[local-name()=$elname]" /> -->
    <xsl:apply-templates select="schema" mode="schema_make_table_body">
      <xsl:with-param name="group" select="$group" />
      <xsl:with-param name="lines" select="$lines" />
    </xsl:apply-templates>
  </xsl:template>
                
  <xsl:template match="*" mode="make_table">
    <xsl:param name="host" select="'page'" />
    
    <xsl:variable name="path">
      <xsl:apply-templates select="." mode="get_path" />
    </xsl:variable>

    <xsl:variable name='schema' select="./schema" />

    <xsl:variable name="title">
      <xsl:choose>
        <xsl:when test="$schema/@title"><xsl:value-of select="$schema/@title" /></xsl:when>
        <xsl:when test="/*/@title"><xsl:value-of select="/*/@title" /></xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="string-length($title)&gt;0">
      <xsl:call-template name="write_title">
        <xsl:with-param name="title" select="$title" />
      </xsl:call-template>
    </xsl:if>

    <xsl:element name="table">
      <xsl:call-template name="tag_class">
        <xsl:with-param name="type" select="'default_table'" />
      </xsl:call-template>
      <xsl:attribute name="data-result-type">table</xsl:attribute>
      <xsl:attribute name="data-result-path">
        <xsl:value-of select="$path" />
      </xsl:attribute>
      <xsl:attribute name="class">
        <xsl:text>Schema</xsl:text>
        <xsl:if test="$schema/@table_class">
          <xsl:value-of select="concat(' ',$schema/@table_class)" />
        </xsl:if>
      </xsl:attribute>

      <xsl:variable name="on_line_click">
        <xsl:choose>
          <xsl:when test="$schema/@on_line_click">
            <xsl:value-of select="$schema/@on_line_click" />
          </xsl:when>
          <xsl:when test="/*/@on_line_click">
            <xsl:value-of select="/*/@on_line_click" />
          </xsl:when>
        </xsl:choose>
      </xsl:variable>

      <xsl:if test="$on_line_click">
        <xsl:attribute name="data-on_line_click">
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="$on_line_click" />
          </xsl:call-template>
        </xsl:attribute>
      </xsl:if>
      <thead>
        <xsl:if test="$host='page'">
          <xsl:apply-templates select="$schema" mode="make_table_head" >
            <xsl:with-param name="class" select="'floater'" />
          </xsl:apply-templates>
        </xsl:if>
        <xsl:apply-templates select="$schema" mode="make_table_head" />
      </thead>
      <tbody>
        <xsl:apply-templates select="." mode="make_table_body" />
      </tbody>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema" mode="make_table_head">
    <xsl:param name="class" />

    <xsl:apply-templates select="." mode="show_intro">
      <xsl:with-param name="class" select="$class" />
    </xsl:apply-templates>

    <xsl:apply-templates select="." mode="show_buttons">
      <xsl:with-param name="class" select="$class" />
    </xsl:apply-templates>

    <xsl:element name="tr">
      <xsl:attribute name="class">
        <xsl:text>headfix_cheads</xsl:text>
        <xsl:if test="$class">
          <xsl:value-of select="concat(' ',$class)" />
        </xsl:if>
      </xsl:attribute>
      <xsl:apply-templates select="field" mode="make_column_head" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_column_head_class">
    <xsl:if test="not(@unsortable)">sortable</xsl:if>
    <xsl:if test="@class">
      <xsl:value-of select="concat(' ',@class)" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema/field[not(@hidden)]" mode="make_column_head">
    <xsl:variable name="name" select="@name" />
    <xsl:variable name="label">
      <xsl:apply-templates select="." mode="get_label" />
    </xsl:variable>
    <xsl:variable name="class">
      <xsl:apply-templates select="." mode="get_column_head_class" />
    </xsl:variable>
    
    <xsl:element name="th">
      <xsl:attribute name="data-name">
        <xsl:value-of select="$name" />
      </xsl:attribute>
      
      <xsl:attribute name="data-type">
        <xsl:value-of select="@nodetype" />
      </xsl:attribute>
      
      <xsl:if test="$class">
        <xsl:attribute name="class">
          <xsl:value-of select="$class" />
        </xsl:attribute>
      </xsl:if>
      
      <xsl:value-of select="$label" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema/field[@row_class]" mode="get_row_class">
    <xsl:param name="data" />
    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>
    <xsl:if test="$value=@row_class">
      <xsl:value-of select="concat(' ',@name)" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="make_table_line">
    <xsl:param name="line_id" />

    <xsl:variable name="schema" select="../schema" />

    <!-- for ad-hoc lines, figure id_field on the fly -->    
    <xsl:variable name="id_field">
      <xsl:choose>
        <xsl:when test="$line_id">
          <xsl:value-of select="$line_id" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="$schema" mode="get_id_field_name" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="row_class">
      <xsl:apply-templates select="$schema/field" mode="get_row_class">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>
v    </xsl:variable>
    
    <xsl:variable name="row_class_flag" select="$schema/@row_class_flag" />

    <xsl:element name="tr">
      <xsl:if test="string-length($row_class) &gt; 0">
        <xsl:attribute name="class">
          <xsl:value-of select="$row_class" />
        </xsl:attribute>
      </xsl:if>

      <xsl:if test="@show_and_highlight">
        <xsl:attribute name="id">schema_target_line</xsl:attribute>
      </xsl:if>

      <!-- <xsl:if test="$row_class_flag"> -->
      <!--   <xsl:if test="boolean(@*[name()=$row_class_flag])"> -->
      <!--     <xsl:attribute name="class"> -->
      <!--       <xsl:value-of select="$schema/@flagged_row_class" /> -->
      <!--     </xsl:attribute> -->
      <!--   </xsl:if> -->
      <!-- </xsl:if> -->
      
      <xsl:if test="$id_field">
        <xsl:attribute name="data-id">
          <xsl:value-of select="@*[name()=$id_field]" />
        </xsl:attribute>
      </xsl:if>

      <xsl:apply-templates select="$schema/*" mode="make_line_cell">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema/field[not(@hidden or @ignore)]" mode="make_line_cell">
    <xsl:param name="data" />
    <xsl:element name="td">
      
      <xsl:if test="@cell-class">
        <xsl:attribute name="class">
          <xsl:value-of select="@cell-class" />
        </xsl:attribute>
      </xsl:if>
      
      <xsl:apply-templates select="." mode="fill_table_cell">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <!-- Utility templates -->
  <xsl:template match="group/field" mode="make_field_gid">
    <xsl:variable name="field" select="../../field[@name=current()/@name]" />
    <xsl:if test="$field">
      <xsl:value-of select="concat('-',generate-id($field))" />
    </xsl:if>
  </xsl:template>

  <!-- Returns dash-delimited list of gids for each schema field element in a group -->
  <xsl:template match="group" mode="get_field_gids">
    <xsl:variable name="t">
      <xsl:apply-templates select="field"
                           mode="make_field_gid" />
    </xsl:variable>
    <xsl:value-of select="substring($t,2)" />
  </xsl:template>

  <xsl:template match="schema" mode="get_field_gids">
    <xsl:variable name="t">
      <xsl:apply-templates select="fields" mode="make_field_gid" />
    </xsl:variable>
    <xsl:value-of select="substring($t,2)" />
  </xsl:template>

  <xsl:template name="next_gid">
    <xsl:param name="gids" />
    <xsl:if test="string-length($gids) &gt; 0">
      <xsl:choose>
        <xsl:when test="contains($gids,'-')">
          <xsl:value-of select="substring-before($gids,'-')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$gids" />
        </xsl:otherwise>
      </xsl:choose>
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

  <xsl:template name="make_result_column_head">
    <xsl:param name="gids" />
    
    <xsl:variable name="gid">
      <xsl:call-template name="next_gid">
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:if test="string-length($gid) &gt; 0">
      <xsl:variable name="field" select="/*/schema/field[generate-id()=$gid]" />
      <xsl:variable name="label">
        <xsl:apply-templates select="$field" mode="get_label" />
      </xsl:variable>
      
      <xsl:if test="not($field/@hidden or $field/@ignore)">
        <th>
          <xsl:value-of select="$label" />
        </th>
      </xsl:if>
      
      <xsl:call-template name="make_result_column_head">
        <xsl:with-param name="gids" select="substring-after($gids,'-')"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="make_gid_table_head">
    <xsl:param name="gids" />
    <tr>
      <xsl:call-template name="make_result_column_head">
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </tr>
  </xsl:template>

  <xsl:variable name="datetimetypes" select="'#DATE#TIME#DATETIME#NEWDATE#'" />

  <xsl:template match="field" mode="make_result_td_type">
    <xsl:variable name="type" select="concat('#',@type,'#')" />
    <xsl:if test="contains($datetimetypes, $type)">
      <xsl:variable name="lower" select="translate(@type,'ADEIMTW','adeimtw')" />
      <xsl:value-of select="concat('dtp_', $lower)" />
    </xsl:if>
  </xsl:template>


  <xsl:template name="make_result_td">
    <xsl:param name="gids" />
    <xsl:param name="data" />

    <xsl:variable name="gid">
      <xsl:call-template name="next_gid">
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="group" select="$data/ancestor::*[schema]" />

    <xsl:if test="string-length($gid) &gt; 0">
      <xsl:variable name="field" select="$group/schema/field[generate-id()=$gid]" />

      <xsl:variable name="type">
        <xsl:apply-templates select="$field" mode="make_result_td_type" />
      </xsl:variable>

      <xsl:if test="not($field/@hidden or $field/@ignore)">
        <xsl:element name="td">
          <xsl:if test="$type and string-length($type)&gt;0">
            <xsl:attribute name="data-type">
              <xsl:value-of select="$type" />
            </xsl:attribute>
          </xsl:if>
          <xsl:apply-templates select="$field" mode="fill_group_cell">
            <xsl:with-param name="data" select="$data" />
          </xsl:apply-templates>
        </xsl:element>
      </xsl:if>
      
      <xsl:call-template name="make_result_td">
        <xsl:with-param name="gids" select="substring-after($gids,'-')"/>
        <xsl:with-param name="data" select="$data" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="make_result_tr">
    <xsl:param name="group" />
    <xsl:param name="primary_key" />
    <xsl:param name="gids" />
    
    <xsl:variable name="recid">
      <xsl:if test="$group and $primary_key">
        <xsl:apply-templates select="$group" mode="get_value">
          <xsl:with-param name="data" select="." />
          <xsl:with-param name="field_name" select="$primary_key" />
        </xsl:apply-templates>
      </xsl:if>
    </xsl:variable>

    <xsl:element name="tr">

      <xsl:if test="$recid">
        <xsl:attribute name="data-id">
          <xsl:value-of select="$recid" />
        </xsl:attribute>
      </xsl:if>

      <xsl:call-template name="make_result_td">
        <xsl:with-param name="data" select="." />
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </xsl:element>
  </xsl:template>

  <xsl:template match="message/@*">
    <dt><xsl:value-of select="local-name()" /></dt>
    <dd><xsl:value-of select="." /></dd>
  </xsl:template>

  <xsl:template match="message">
    <h2>Message</h2>
    <dl>
    <xsl:apply-templates select="@*" />
    </dl>
  </xsl:template>

  <xsl:template name="make_tableEdit_colTypesString">
    <xsl:param name="gids" />
    <xsl:param name="first" select="1" />
    
    <xsl:variable name="gid">
      <xsl:call-template name="next_gid">
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:if test="string-length($gid) &gt; 0">
      <xsl:variable name="field" select="/*/schema/field[generate-id()=$gid]" />
      <xsl:if test="$first=0">|</xsl:if>
      <xsl:value-of select="$field/@name" />

      <xsl:call-template name="make_tableEdit_colTypesString">
        <xsl:with-param name="gids" select="substring-after($gids,'-')" />
        <xsl:with-param name="first" select="0" />
      </xsl:call-template>
    </xsl:if>

  </xsl:template>


  <xsl:template match="schema/group" mode="get_primary_key">
    <xsl:variable name="goop" select="../field[@name=current()/field/@name]" />
    <xsl:value-of select="$goop[@primary-key]/@name" />
  </xsl:template>

  <xsl:template match="schema/group" mode="make_result_table">
    <xsl:param name="data" />

    <xsl:variable name="els" select="$data/*[local-name()=current()/@name]" />

    <xsl:variable name="primary_key">
      <xsl:apply-templates select="." mode="get_primary_key" />
    </xsl:variable>

    <xsl:variable name="gids">
      <xsl:apply-templates select="." mode="get_field_gids" />
    </xsl:variable>

    <xsl:variable name="colstring">
      <xsl:call-template name="make_tableEdit_colTypesString">
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </xsl:variable>

   <xsl:element name="table">
      <xsl:attribute name="data-result_name">
        <xsl:value-of select="@name" />
      </xsl:attribute>
      <xsl:apply-templates select="@*" mode="add_html_attributes" />
      <thead>
        <xsl:if test="@allow-add or @allow-delete">
          <tr class="action_bar">
            <th colspan="99">
              <xsl:if test="@allow-add">
                <a href="javascript:void(0)"
                   tabindex="-1"
                   data-action="add_row"
                   title="Ctrl-Alt-plus or Ctrl-Alt-equals">Add Row</a>
              </xsl:if>
              <xsl:if test="@allow-delete">
                <a href="javascript:void(0)"
                   tabindex="-1"
                   class="active-only"
                   data-action="delete_row"
                   title="Ctrl-Alt-minus">Delete Row</a>
              </xsl:if>
            </th>
          </tr>
        </xsl:if>
        <xsl:call-template name="make_gid_table_head">
          <xsl:with-param name="gids" select="$gids" />
        </xsl:call-template>
      </thead>
      <tbody>
        <xsl:if test="count($els)=0">
          <xsl:apply-templates select="." mode="make_result_tr">
            <xsl:with-param name="group" select="." />
            <xsl:with-param name="primary_key" select="$primary_key" />
            <xsl:with-param name="gids" select="$gids" />
          </xsl:apply-templates>
        </xsl:if>
        <xsl:apply-templates select="$els" mode="make_result_tr">
          <xsl:with-param name="group" select="." />
          <xsl:with-param name="primary_key" select="$primary_key" />
          <xsl:with-param name="gids" select="$gids" />
        </xsl:apply-templates>
      </tbody>
    </xsl:element>
  </xsl:template>


  <!-- Use attribute to branch to appropriate template: -->
  <xsl:template match="*[@make_table_body][schema]">
    <xsl:apply-templates select="." mode="make_table_body" />
  </xsl:template>

  <xsl:template match="*[@make_table_line][../schema]">
    <xsl:apply-templates select="." mode="make_table_line" />
  </xsl:template>

  <!-- The next two templates should be deleted when the function
       OLD_build_dialog is finally deleted.  They won't be necessary
       when we commit to NEW_build_dialog, which is simpler and more
       flexible.
       -->

  <xsl:template match="*[@make_dialog][schema]">
    <xsl:variable name="elname" select="schema/@name" />
    <xsl:variable name="el" select="./*[local-name()=$elname]" />

    <xsl:choose>
      <xsl:when test="@mode-type='form-edit'">
        <xsl:variable name="row-name" select="schema/@name" />
        
        <xsl:apply-templates select="schema" mode="make_form" />

        <!-- <xsl:apply-templates select="schema" mode="make_form"> -->
        <!--   <xsl:with-param name="data" select="*[@rndx='1']/*[position()=1]" /> -->
        <!-- </xsl:apply-templates> -->
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="schema" mode="make_form">
          <xsl:with-param name="data" select="$el" />
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@make_node_dialog]">
    <xsl:variable name="data-row" select="*[not(self::schema)][1]" />

    <xsl:variable name="form-schema" select="../schema" />
    <xsl:variable name="result-schema" select="schema" />

    <xsl:apply-templates select="$form-schema" mode="make_form">
      <xsl:with-param name="data" select="$data-row" />
      <xsl:with-param name="result-schema" select="$result-schema" />
    </xsl:apply-templates>
  </xsl:template>

</xsl:stylesheet>
