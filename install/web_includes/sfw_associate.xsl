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

  <xsl:template match="field[@type='assoc'][@result]">

    <xsl:variable name="an_result" select="ancestor::*[@merged]" />
    <xsl:variable name="so_result"
                  select="/*/*[local-name($an_result)='schema'][@rndx][@merged=$an_result/@merged]" />

    <xsl:variable name="sresult" select="($an_result|$so_result)[not(@type='update')]" />
    <xsl:variable name="row" select="$sresult/*[local-name()=$sresult/@row-name]" />

    <xsl:apply-templates select="$row" mode="build_with_associated_row">
      <xsl:with-param name="field" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <!-- This should a field in a non-merged result/schema for the root-document.
       It is likely for a table, and should get a nodeset rather than a node.
  -->
  <xsl:template match="field[@type='assoc'][@result][@data-id][not(ancestor::*/@merge-type)]">

    <xsl:variable name="idname">
      <xsl:apply-templates select=".." mode="get_id_field_name" />
    </xsl:variable>
    <xsl:variable name="rowname" select="ancestor::*[@rndx]/@row-name" />
    <xsl:variable name="result" select="ancestor::*[@rndx]" />
    <xsl:variable name="row" select="$result/*[@*[local-name()=$idname] = current()/@data-id]" />

    <xsl:apply-templates select="$row" mode="build_with_associated_row">
      <xsl:with-param name="field" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*" mode="build_with_associated_row">
    <xsl:param name="field" />

    <xsl:variable name="alist">
      <xsl:apply-templates select="$field" mode="get_association_list">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="dresult" select="/*/*[@rndx][local-name()=$field/@result]" />
    <xsl:variable name="aresult" select="/*/*[local-name()=$dresult[@type='association']/@result]" />

    <xsl:call-template name="transform_associated_references">
      <xsl:with-param name="result" select="$aresult|$dresult[not($aresult)]" />
      <xsl:with-param name="field" select="$field" />
      <xsl:with-param name="str" select="$alist" />
    </xsl:call-template>
  </xsl:template>


  <!-- <xsl:template match="field[@type='assoc'][@result]"> -->

  <!--   <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" /> -->
  <!--   <xsl:variable name="row" select="$result/*[local-name()=$result/@row-name]" /> -->

  <!--   <xsl:apply-templates select="." mode="build_associated_row"> -->
  <!--     <xsl:with-param name="row" -->
  <!--                     select="$result/*[local-name()=$result/@row-name]" /> -->
  <!--   </xsl:apply-templates> -->
  <!-- </xsl:template> -->

  <!-- Call this template when building a host to assoc-type fields so the
       framework can find the approriate field and host element for updates. -->
  <xsl:template match="field[@type='assoc']" mode="add_assoc_attribute">
    <xsl:attribute name="data-sfw-assoc">
      <xsl:value-of select="@name" />
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="field[@type='assoc']" mode="add_class">
    <xsl:variable name="cname">
      <xsl:text>assoc</xsl:text>
      <xsl:if test="@active"> active</xsl:if>
    </xsl:variable>
    <xsl:attribute name="class"><xsl:value-of select="$cname" /></xsl:attribute>
  </xsl:template>

  <xsl:template match="field[@type='assoc']" mode="construct_input">
    <xsl:param name="data" />
    <xsl:element name="div">
      <xsl:apply-templates select="." mode="add_class" />
      <xsl:if test="@active">
        <xsl:attribute name="data-sfw-class">assoc</xsl:attribute>
        <xsl:attribute name="data-sfw-input">assoc</xsl:attribute>
        <xsl:apply-templates select="." mode="add_assoc_attribute" />
        <xsl:apply-templates select="@*" mode="add_on_click_attribute" />
      </xsl:if>
      
      <xsl:apply-templates select="." mode="build_associated_row">
        <xsl:with-param name="row" select="$data" />
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@type='assoc'][@style='table']" mode="construct_input">
    <xsl:param name="data" />

    <xsl:if test="buttons">
      <xsl:apply-templates select="buttons" mode="construct_buttons">
        <xsl:with-param name="host-type" select="'div'" />
      </xsl:apply-templates>
    </xsl:if>

    <xsl:element name="table">
      <xsl:apply-templates select="." mode="add_class" />
      <xsl:if test="@active">
        <xsl:attribute name="data-sfw-class">assoc</xsl:attribute>
        <xsl:attribute name="data-sfw-input">input</xsl:attribute>
        <xsl:apply-templates select="@*" mode="add_on_click_attribute" />
      </xsl:if>
      <xsl:element name="tbody">
        <xsl:apply-templates select="." mode="add_assoc_attribute" />
        <xsl:apply-templates select="." mode="build_associated_row">
          <xsl:with-param name="row" select="$data" />
        </xsl:apply-templates>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <!-- Returns a csv string of integer values that reference elements of another result.
  -->
  <xsl:template match="field[@type='assoc']" mode="get_association_list">
    <xsl:param name="data" />

    <xsl:variable name="aresult" select="/*/*[local-name()=current()/@result]" />

    <xsl:choose>
      <xsl:when test="not($aresult)">Failed to find associated result.</xsl:when>
      <xsl:when test="$aresult[@type='association']">
        <xsl:variable name="idval">
          <xsl:apply-templates select="." mode="get_id_value">
            <xsl:with-param name="data" select="$data" />
          </xsl:apply-templates>
        </xsl:variable>

        <xsl:variable name="krow" select="key(@xslkey, $idval)" />
        <xsl:variable name="drow"
                      select="$aresult[not($krow)]/*[local-name()=$aresult/@row-name][@id=$idval]" />

        <xsl:variable name="row" select="$krow|$drow" />

        <xsl:value-of select="$row/@*[local-name()=current()/@name]" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$data/@*[local-name()=current()/@name]" />
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>

  <xsl:template match="field[@type='assoc']" mode="final_build_associated_row">
    <xsl:param name="row" />

    <xsl:variable name="alist">
      <xsl:apply-templates select="." mode="get_association_list">
        <xsl:with-param name="data" select="$row" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="ref_result" select="/*/*[local-name()=current()/@result]" />
    <xsl:variable name="aaresult"
                  select="/*/*[local-name()=$ref_result[@type='association']/@result]" />

    <xsl:call-template name="transform_associated_references">
      <xsl:with-param name="result" select="$aaresult|$ref_result[not($aaresult)]" />
      <xsl:with-param name="field" select="." />
      <xsl:with-param name="str" select="$alist" />
    </xsl:call-template>

  </xsl:template>

  <xsl:template match="field[@type='assoc']" mode="build_associated_row">
    <xsl:param name="row" />
    <xsl:apply-templates select="." mode="final_build_associated_row">
      <xsl:with-param name="row" select="$row" />
    </xsl:apply-templates>
  </xsl:template>

  <!-- Handles special case of assoc contents in a table cell rather row. -->
  <xsl:template match="field[@type='assoc'][@data-id]" mode="build_associated_row">
    <xsl:param name="row" />
    <xsl:variable name="idname">
      <xsl:apply-templates select=".." mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="srow"
                  select="$row[@*[local-name()=$idname]=current()/@data-id]" />

    <xsl:apply-templates select="." mode="final_build_associated_row">
      <xsl:with-param name="row" select="$srow" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field[@type='assoc'][@style='table']" mode="construct_assoc_input">

    <!-- Get parts from construct_input that work with transform_row -->
    <div>Ha...got ya!  <xsl:value-of select="$source_result/@row-name" /></div>
    <table class="Schema">
       <tbody>
         <xsl:apply-templates select="." mode="transform_row">
           <xsl:with-param name="data" select="$data" />
         </xsl:apply-templates>
       </tbody>
    </table>
  </xsl:template>

  <!--
  Primary template for rendering the associated contents of a field.
  The query that includes this field should select a NULL value for the
  the field name and an appropriate schema/field instruction.

  For rows with multiple non-id attributes, include a template
  instruction to the field instruction to dictate which attributes
  to include and in what order.
  -->
  <xsl:template match="field[@type='assoc']" mode="show_associations">
    <xsl:param name="data" />

    <xsl:variable name="lresult" select="/*/*[local-name()=current()/@result]" />

    <xsl:variable name="idval">
      <xsl:apply-templates select="." mode="get_id_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <!-- <xsl:choose> -->
    <!--   <xsl:when test="$lresult/@xslkey and not($lresult/@xslkey='auto')"> -->


    <!--     <xsl:apply-templates select="key($lresult/@xslkey, $idval)" mode="build_assoc"> -->
    <!--       <xsl:with-param name="field" select="." /> -->
    <!--     </xsl:apply-templates> -->

    <!--   </xsl:when> -->
    <!--   <xsl:otherwise> -->

    <!--     <xsl:apply-templates -->
    <!--         select="$lresult/*[local-name()=../@row-name][@id=$idval]" -->
    <!--         mode="build_assoc"> -->
    <!--       <xsl:with-param name="field" select="." /> -->
    <!--     </xsl:apply-templates> -->


    <!--     <xsl:apply-templates select="." mode="build_with_row"> -->
    <!--       <xsl:with-param name="row" select="$lresult/*[local-name()=../@row-name][@id=$idval]" /> -->
    <!--     </xsl:apply-templates> -->
        
    <!--   </xsl:otherwise> -->
    <!-- </xsl:choose> -->

    <span><xsl:value-of select="concat('idval=',$idval)" /></span>


    <xsl:apply-templates select="." mode="build_associated_row">
      <xsl:with-param name="row" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*[@rndx][@result]/*" mode="build_assoc">
    <xsl:param name="field" />

    <xsl:text>build-assoc</xsl:text>
  </xsl:template>



  <!-- Default transform_row template.
       Override this template with a following template with more specific field match.
       -->
  <xsl:template match="field[@type='assoc'][@style='table']" mode="transform_row">
    <xsl:param name="id" />
    <xsl:param name="row" />

    <xsl:element name="tr">
      <xsl:if test="$id">
        <xsl:attribute name="data-id"><xsl:value-of select="$id" /></xsl:attribute>
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@template" />
          <xsl:with-param name="row" select="$row" />
          <xsl:with-param name="enclose" select="'td'" />
        </xsl:call-template>
      </xsl:if>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@type='assoc'][@style='span']" mode="transform_row">
    <xsl:param name="id" />
    <xsl:param name="row" />

    <xsl:element name="span">
      <xsl:if test="$id">
        <xsl:attribute name="data-id"><xsl:value-of select="$id" /></xsl:attribute>
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@template" />
          <xsl:with-param name="row" select="$row" />
        </xsl:call-template>
      </xsl:if>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@type='assoc'][@style='csv']" mode="transform_row">
    <xsl:param name="id" />
    <xsl:param name="row" />
    <xsl:param name="more" />

    <xsl:variable name="val">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="@template" />
        <xsl:with-param name="row" select="$row" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="normalize-space($val)" />

    <xsl:if test="$more">, </xsl:if>
  </xsl:template>

  <xsl:template name="transform_associated_references">
    <xsl:param name="result" />
    <xsl:param name="field" />
    <xsl:param name="str" />

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean(string-length($c_id)=0))" />
    <xsl:variable name="id_val" select="concat($c_id,$s_id)" />

    <xsl:variable name="more" select="string-length($c_id)&gt;0" />

    <xsl:variable name="row" select="$result/*[local-name()=../@row-name][@id=$id_val]" />

    <xsl:apply-templates select="$field" mode="transform_row">
      <xsl:with-param name="id" select="$id_val" />
      <xsl:with-param name="row" select="$row" />
      <xsl:with-param name="more" select="$more" />
    </xsl:apply-templates>
    
    <xsl:if test="$more">
      <xsl:call-template name="transform_associated_references">
        <xsl:with-param name="result" select="$result" />
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="str" select="substring-after($str,',')" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>


  <xsl:template match="*[parent::*[@rndx][@type='ref-lists']]" mode="resolve_references">
    <xsl:param name="field" />

    <xsl:call-template name="transform_associated_references">
      <xsl:with-param name="result" select=".." />
      <xsl:with-param name="field" select="$field" />
      <xsl:with-param name="str" select="@*[local-name()=$field/@name]" />
    </xsl:call-template>

  </xsl:template>

  <!-- Helper template to get a CSV list of integers from the association table/element. -->
  <xsl:template match="field[@type='assoc']" mode="get_int_list">
    <xsl:param name="data" />

    <xsl:variable name="id_value">
      <xsl:apply-templates select="." mode="get_id_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="lresult" select="/*/*[local-name()=current()/@result]" />
    <xsl:variable name="associated_row" select="$lresult/*[local-name()=../@row-name][@id=$id_value]" />

    <xsl:value-of select="$associated_row/@*[local-name()=current()/@name]" />
  </xsl:template>

  <!-- Special handling for associated fields, to override matching template in sfw_form.xsl. -->
  <xsl:template match="field[@type='assoc']" mode="display_value">
    <xsl:param name="data" />
  </xsl:template>

    <!-- Special handling for associated fields, to override matching template in sfw_form.xsl. -->
  <xsl:template match="field[@type='assoc'][@style='table']" mode="display_value">
    <xsl:param name="data" />
    <table>
      <xsl:apply-templates select="." mode="show_associations">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </table>
  </xsl:template>


</xsl:stylesheet>
