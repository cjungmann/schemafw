<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">


  <xsl:template match="@*" mode="enum_attr">
    <xsl:value-of select="concat(' ', local-name(), '=', $apos, string(.), $apos)" />
  </xsl:template>

  <xsl:template match="*" mode="show_element">
    <p>
      <xsl:value-of select="concat('for ', $apos,local-name(),$apos)" />
      <xsl:apply-templates select="@*" mode="enum_attr" />
    </p>
  </xsl:template>


  <!-- Entry-point templates -->
  <xsl:template match="field[@type='linked']" mode="write_cell_content">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="construct_linked_field">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field[@type='linked']" mode="construct_input">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="construct_linked_field">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field[@type='linked']" mode="construct_linked_field">
    <xsl:param name="data" />

    <xsl:variable name="v_str">
      <xsl:if test="@virtual">
        <xsl:apply-templates select="$data" mode="get_id_from_row" />
      </xsl:if>
    </xsl:variable>
    <xsl:variable name="d_str" select="$data[not($v_str)]/@*[local-name()=current()/@name]" />

    <xsl:variable name="rname" select="link/@result" />
    <xsl:variable name="result" select="/*/*[local-name()=$rname]" />

    <xsl:apply-templates select="buttons" mode="construct_buttons" />

    <xsl:apply-templates select="$result" mode="parse_result_links">
      <xsl:with-param name="str" select="concat($v_str,$d_str)" />
      <xsl:with-param name="link" select="link" />
      <xsl:with-param name="idname">
        <xsl:apply-templates select="$result" mode="lookup_id_name" />
      </xsl:with-param>
    </xsl:apply-templates>
  </xsl:template>

  <!-- Warnings for incomplete linked fields -->
  <xsl:template match="field[@type='linked'][not(link/@result)]" mode="construct_linked_field">
     <span>Linked field without link instructions.</span>
  </xsl:template>



  <!-- Integer-string parsing templates -->

  <xsl:template match="*[@rndx]" mode="parse_result_links">
    <xsl:param name="str" />
    <xsl:param name="link" />
    <xsl:param name="idname" />

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="c_id_len" select="string-length($c_id)" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean($c_id_len=0))" />
    <xsl:variable name="r_id" select="concat($c_id,$s_id)" />

    <xsl:apply-templates select="." mode="use_linked_result">
      <xsl:with-param name="id" select="$r_id" />
      <xsl:with-param name="link" select="$link" />
    </xsl:apply-templates>

    <xsl:if test="$c_id_len">
      <xsl:apply-templates select="." mode="process_result_links">
        <xsl:with-param name="str" select="substring-after($str,',')" />
        <xsl:with-param name="result" select="." />
        <xsl:with-param name="idname" select="$idname" />
      </xsl:apply-templates>
    </xsl:if>

  </xsl:template>


  <!-- Directly use the data -->
  <xsl:template match="*" mode="use_linked_row">
    <xsl:param name="link" />
    <span>(end) Got here!</span>
  </xsl:template>

  <!-- Result is link to another result -->
  <xsl:template match="*[../@links]" mode="use_linked_row">
    <xsl:param name="link" />

    <span>(links) Got here!</span>

    <xsl:variable name="rname" select="../@result" />
    <xsl:variable name="result" select="/*/*[local-name()=$rname]" />

    <xsl:apply-templates select="$result" mode="parse_result_links">
      <xsl:with-param name="str" select="string(@*[2])" />
      <xsl:with-param name="link" select="$link" />
      <xsl:with-param name="idname">
        <xsl:apply-templates select="$result" mode="lookup_id_name" />
      </xsl:with-param>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*[not(../@links) or not(../@result)]" mode="use_linked_row">
    <span>Mode process_linked_row requires both links and result attributes.</span>
  </xsl:template>












  <xsl:template match="*[@rndx]" mode="use_linked_result">
    <xsl:param name="id" />
    <xsl:param name="link" />

    <xsl:variable name="row" select="*[local-name()=../@row-name][@*[1]=$id]" />

    <p>calling use_linked_row from result without schema</p>
    <xsl:apply-templates select="$row" mode="use_linked_row">
      <xsl:with-param name="link" select="$link" />
    </xsl:apply-templates>

  </xsl:template>

  <xsl:template match="*[@rndx][schema]" mode="use_linked_result">
    <xsl:param name="id" />
    <xsl:param name="link" />

    <xsl:variable name="idname">
      <xsl:apply-templates select="schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="row" select="*[local-name()=../@row-name][@*[local-name()=$idname]=$id]" />

    <p>calling use_linked_row from result with schema</p>
    <xsl:apply-templates select="$row" mode="use_linked_row">
      <xsl:with-param name="link" select="$link" />
    </xsl:apply-templates>

  </xsl:template>

  <xsl:template match="*[@rndx]" mode="parse_links_list">
    <xsl:param name="link" />
    <xsl:param name="str" />

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean(string-length($c_id)=0))" />

    <xsl:apply-templates select="." mode="use_linked_result">
      <xsl:with-param name="id" select="concat($c_id,$s_id)" />
      <xsl:with-param name="link" select="$link" />
    </xsl:apply-templates>

    <xsl:if test="string-length($s_id)=0">
      <xsl:apply-templates select="." mode="parse_links_list">
        <xsl:with-param name="link" select="$link" />
        <xsl:with-param name="str" select="substring-after($str,',')" />
      </xsl:apply-templates>
    </xsl:if>

  </xsl:template>

  <xsl:template match="*[local-name()=../@row-name]" mode="use_linked_row">
    <xsl:param name="link" />
    <p>about to use_linked_row from use_linked_row</p>
    <xsl:apply-templates select="$link" mode="use_linked_row">
      <xsl:with-param name="row" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*[local-name()=../@row-name][../@type='link']"
                mode="use_linked_row">
    <xsl:param name="link" />
  </xsl:template>

  

  <xsl:template match="link" mode="construct_row">
    <xsl:param name="row" />
    <xsl:variable name="val" select="$row/@*[2]" />

    <!-- branch on $link/@style, with a special case for @style='table' -->
    <xsl:value-of select="concat('construct_row matched link...style=', @style, ', val=', $val)" />
    <xsl:apply-templates select="." mode="show_element" />
  </xsl:template>

  <xsl:template match="link[@template][@style='csv']" mode="construct_row">
    <xsl:param name="row" />

    <xsl:value-of select="concat('template = ', $apos, @template, $apos)" />
    <xsl:apply-templates select="$row" mode="show_element" />

    <xsl:variable name="val">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="@template" />
        <xsl:with-param name="row" select="$row" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="$val" />
    
  </xsl:template>



  <xsl:template match="*[../@rndx]" mode="use_linked_row">
    <xsl:param name="link" />

    <p>use_linked_row: About to directly use a linked row.</p>

    <xsl:apply-templates select="$link" mode="construct_row">
      <xsl:with-param name="row" select="." />
    </xsl:apply-templates>

  </xsl:template>

  
  <xsl:template match="*[../@result][../@links]" mode="use_linked_row">
    <xsl:param name="link" />

    <xsl:variable name="rname" select="../@result" />
    <xsl:variable name="result" select="/*/*[local-name()=$rname]" />

    <xsl:apply-templates select="$result" mode="parse_result_links">
      <xsl:with-param name="str" select="string(@*[2])" />
      <xsl:with-param name="link" select="$link" />
      <xsl:with-param name="idname">
        <xsl:apply-templates select="$result" mode="lookup_id_name" />
      </xsl:with-param>
    </xsl:apply-templates>

  </xsl:template>

  <!-- Warning for badly-formed links row. -->
  <xsl:template match="*[../@links][not(count(@*)=2)]" mode="use_linked_row">Links Result rows must have exactly 2 attributes.</xsl:template>
  <!-- Warning for badly-formed links result. -->
  <xsl:template match="*[../@links][not(../@result)]" mode="use_linked_row">Links Result missing results attribute.</xsl:template>






  <xsl:template match="*" mode="get_id_from_row">
    <xsl:variable name="sname">
      <xsl:apply-templates select="../schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$sname">
        <xsl:value-of select="@*[local-name()=$sname]" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@*[1]" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="lookup_id_name">
    <xsl:variable name="fname">
      <xsl:apply-templates select="schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="attr" select="*[not($fname)][../@row-name]/@*[1]" />

    <xsl:choose>
      <xsl:when test="$fname"><xsl:value-of select="$fname" /></xsl:when>
      <xsl:when test="$attr"><xsl:value-of select="local-name($attr)" /></xsl:when>
      <xsl:otherwise>id</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="get_attrib_value">
    <xsl:param name="column" />
    <xsl:value-of select="*/@*[local-name()=$column]" />
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="lookup_value">
    <xsl:param name="id" />
    <xsl:param name="column" />

    <xsl:variable name="idname">
      <xsl:apply-templates select="." mode="lookup_id_name" />
    </xsl:variable>

    <xsl:variable name="row" select="*[local-name()=../@row-name/@*[local-name()=$idname]]" />

    <xsl:apply-templates select="$row" mode="get_attrib_value">
      <xsl:with-param name="column" select="$column" />
    </xsl:apply-templates>

  </xsl:template>

  <xsl:template match="*[@rndx][@xslkey]" mode="lookup_value">
    <xsl:param name="id" />
    <xsl:param name="column" />

    <xsl:variable name="row">
      <xsl:apply-templates select="." mode="get_by_xsl_key">
        <xsl:with-param name="id" select="$id" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:apply-templates select="$row" mode="get_attrib_value">
      <xsl:with-param name="column" select="$column" />
    </xsl:apply-templates>
  </xsl:template>

  <!-- Discriminate "build_row_with_field" bases on field attributes -->
  <xsl:template match="field" mode="build_row_with_field">
    <xsl:param name="row" />
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="lookup_callback">
    <xsl:param name="id" />
    <xsl:param name="field" />

    <xsl:variable name="idname">
      <xsl:apply-templates select="." mode="lookup_id_name" />
    </xsl:variable>

    <xsl:variable name="row" select="*[local-name()=../@row-name/@*[local-name()=$idname]]" />

    <xsl:if test="$row">
      <xsl:apply-templates select="$field" mode="build_row_with_field">
        <xsl:with-param name="row" select="$row" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*[@rndx][@xslkey]" mode="lookup_callback">
    <xsl:param name="id" />
    <xsl:param name="field" />

    <xsl:variable name="row">
      <xsl:apply-templates select="." mode="get_by_xsl_key">
        <xsl:with-param name="id" select="$id" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:if test="$row">
      <xsl:apply-templates select="$field" mode="build_row_with_field">
        <xsl:with-param name="row" select="$row" />
      </xsl:apply-templates>
    </xsl:if>

  </xsl:template>


</xsl:stylesheet>
