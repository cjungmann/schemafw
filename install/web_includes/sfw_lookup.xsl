<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!-- <xsl:template match="*"> -->
  <!--   <p>No match</p> -->
  <!-- </xsl:template> -->

  <!-- no-mode entry point for reploting field.  Matches a row that cohabits
       with a schema including a linked field. -->
  <xsl:template match="*[../schema/field[@name=current()/@lookup-field-match]]">
    <xsl:variable name="field" select="../schema/field[@name=current()/@lookup-field-match]" />
    <xsl:apply-templates select="$field" mode="host_linked_field">
      <xsl:with-param name="data" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <!-- Override basic mode="write_cell_content" as entry point for table cells -->
  <xsl:template match="schema/field[@type='linked']" mode="write_cell_content">
    <xsl:param name="data" />

    <xsl:apply-templates select="buttons" mode="construct_buttons" />

    <div>
      <xsl:apply-templates select="." mode="host_linked_field">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </div>
  </xsl:template>

  <!-- Override basic mode="construct_input" as entry point for form fields -->
  <xsl:template match="field[@type='linked']" mode="construct_input">
    <xsl:param name="data" />

    <xsl:apply-templates select="buttons" mode="construct_buttons" />

    <div>
      <xsl:apply-templates select="." mode="host_linked_field">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </div>
  </xsl:template>

  <!-- host_linked_field templates are the opportunity to "wrap" the contents.
       before process_linked_fields template.
       See table-wrapping @style='table' example below. -->
  <xsl:template match="field" mode="host_linked_field">
    <xsl:param name="data" />

    <xsl:apply-templates select="." mode="process_linked_field">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field[display/@style='table']" mode="host_linked_field">
    <xsl:param name="data" />

    <xsl:element name="table">
      <xsl:attribute name="class">lookup</xsl:attribute>
      <xsl:attribute name="data-sfw-class">lookup</xsl:attribute>
      <xsl:attribute name="data-sfw-input">input</xsl:attribute>
      <xsl:attribute name="tabindex">0</xsl:attribute>
      <xsl:apply-templates select="display/@*" mode="add_on_click_attribute" />
      <xsl:attribute name="data-path">
        <xsl:apply-templates select="$data" mode="gen_path" />
      </xsl:attribute>
      <tbody>
        <xsl:apply-templates select="." mode="process_linked_field">
          <xsl:with-param name="data" select="$data" />
        </xsl:apply-templates>
      </tbody>
    </xsl:element>
  </xsl:template>
  
  <!-- process the in-place attribute the hard way, without any indexes. -->
  <xsl:template match="field" mode="process_linked_field">
    <xsl:param name="data" />

    <xsl:variable name="result" select="/*/*[local-name()=current()/@result]" />

    <xsl:apply-templates select="$result" mode="get_result_rows">
      <xsl:with-param name="str" select="$data/@*[local-name()=current()/@name]" />
      <xsl:with-param name="display" select="display" />
    </xsl:apply-templates>

  </xsl:template>

  <!-- Lookup virtual attribute value in linked result, without a key. -->
  <xsl:template match="field[@virtual]" mode="process_linked_field">
    <xsl:param name="data" />

    <xsl:variable name="ctxt_result" select="ancestor::*[@rndx][1]" />

    <!-- get id value of current to find row in target result. -->
    <xsl:variable name="idname">
      <xsl:apply-templates select="$ctxt_result" mode="get_idname" />
    </xsl:variable>

    <xsl:variable name="idval" select="$data/@*[local-name()=$idname]" />

    <xsl:variable name="t_result" select="/*/*[local-name()=current()/@result]" />
    <xsl:variable name="t_idname">
      <xsl:apply-templates select="$t_result" mode="get_idname" />
    </xsl:variable>
    <xsl:variable
        name="row"
        select="$t_result/*[local-name()=../@row-name][@*[local-name()=$t_idname]=$idval]" />

    <xsl:variable name="u_result" select="/*/*[local-name()=$t_result/@result]" />

    <xsl:apply-templates select="$u_result" mode="get_result_rows">
      <xsl:with-param name="str" select="$row/@*[local-name()=current()/@name]" />
      <xsl:with-param name="display" select="display" />
    </xsl:apply-templates>

  </xsl:template>

  <!-- process the extant field the hard way, using indexes. -->
  <xsl:template match="field[../../@xslkey]" mode="process_linked_field">
    <xsl:param name="data" />

    <xsl:variable name="result" select="ancestor::*[@rndx][1]" />

    <xsl:apply-templates select="$result" mode="get_result_rows">
      <xsl:with-param name="str" select="$data/@*[local-name()=current()/@name]" />
      <xsl:with-param name="display" select="display" />
    </xsl:apply-templates>

  </xsl:template>

  <!-- Lookup virtual field value in linked result, USING a key. -->
  <xsl:template match="field[@virtual][../../@xslkey]" mode="process_linked_field">
    <xsl:param name="data" />

    <xsl:variable name="idval">
      <xsl:apply-templates select=".." mode="get_idval">
        <xsl:with-param name="row" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="result" select="ancestor::*[@rndx][1]" />

    <xsl:apply-templates select="../.." mode="use_linked_result">
    </xsl:apply-templates>
  </xsl:template>

  <!-- This is the callback uses the virtual attribute to start the indexed rows search. -->
  <xsl:template match="field[@virtual][../../@xslkey]" mode="use_linked_rows">
    <xsl:param name="rows" />

    <xsl:variable name="t_result_name" select="$rows[1]/../@result" />
    <xsl:variable name="t_result" select="/*/*[local-name()=$t_result_name]" />

    <xsl:apply-templates select="$t_result">
      <xsl:with-param name="str" select="$row/@*[local-name()=current()/@name]" />
      <xsl:with-param name="display" select="display" />
    </xsl:apply-templates>

  </xsl:template>

  <xsl:template match="display" mode="use_linked_rows">
    <xsl:param name="rows" />

    <xsl:apply-templates select="." mode="display_row">
      <xsl:with-param name="row" select="$rows" />
    </xsl:apply-templates>
  </xsl:template>


  <!-- get_result_rows is the heart of the process to resolve references.
       It recursively parses a string of integers to get the targeted rows. -->
  <xsl:template match="*[@rndx]" mode="get_result_rows">
    <xsl:param name="str" />
    <xsl:param name="display" />

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="c_id_len" select="string-length($c_id)" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean($c_id_len=0))" />
    <xsl:variable name="r_id" select="concat($c_id,$s_id)" />

    <xsl:apply-templates select="." mode="retrieve_row">
      <xsl:with-param name="display" select="$display" />
      <xsl:with-param name="idval" select="$r_id" />
    </xsl:apply-templates>

    <xsl:if test="$c_id_len&gt;0">
      <xsl:apply-templates select="$display" mode="separate_following" />

      <xsl:apply-templates select="." mode="get_result_rows">
        <xsl:with-param name="str" select="substring-after($str,',')" />
        <xsl:with-param name="display" select="$display" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>
  

  <xsl:template match="display[@style='csv']" mode="separate_following">
    <xsl:text>, </xsl:text>
  </xsl:template>

  <!-- The retrieve_row templates get a row, with or without a key index,
       according to whether or not the result includes an xslkey attribute
       indicating the key template to use. -->
  <xsl:template match="*[@rndx]" mode="retrieve_row">
    <xsl:param name="display" />
    <xsl:param name="idval" />

    <xsl:variable name="idname">
        <xsl:apply-templates select="." mode="get_idname" />
    </xsl:variable>

    <xsl:variable
        name="row"
        select="*[@*[local-name()=$idname]=$idval][local-name()=../@row-name]" />

    <xsl:apply-templates select="$display" mode="use_linked_rows">
      <xsl:with-param name="rows" select="$row" />
    </xsl:apply-templates>

  </xsl:template>

  <xsl:template match="*[@rndx][@xslkey]" mode="retrieve_row">
    <xsl:param name="display" />
    <xsl:param name="idval" />

    <!-- use_linked_result jumps out, to return to the local use_linked_rows template -->
    <xsl:apply-templates select="." mode="use_linked_result">
      <xsl:with-param name="id" select="$idval" />
      <xsl:with-param name="link" select="$display" />
    </xsl:apply-templates>
  </xsl:template>


  <xsl:template match="*[@rndx]" mode="use_linked_result">
    <xsl:param name="id" />
    <xsl:param name="link" />

    <xsl:variable name="row" select="*[local-name()=../@row-name][@*[1]=$id]" />

    <xsl:apply-templates select="$link" mode="use_linked_rows">
      <xsl:with-param name="rows" select="$row" />
    </xsl:apply-templates>

  </xsl:template>

  <xsl:template match="*[@rndx][schema]" mode="use_linked_result">
    <xsl:param name="id" />
    <xsl:param name="link" />

    <xsl:variable name="idname">
      <xsl:apply-templates select="schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="row" select="*[local-name()=../@row-name][@*[local-name()=$idname]=$id]" />

    <xsl:apply-templates select="$link" mode="use_linked_rows">
      <xsl:with-param name="rows" select="$row" />
    </xsl:apply-templates>

  </xsl:template>


  <xsl:template match="display[@style='table']" mode="display_row">
    <xsl:param name="row" />

    <xsl:element name="tr">
      <xsl:if test="$row/@id">
        <xsl:attribute name="data-id"><xsl:value-of select="$row/@id" /></xsl:attribute>
      </xsl:if>
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="@template" />
        <xsl:with-param name="row" select="$row" />
        <xsl:with-param name="enclose" select="'td'" />
      </xsl:call-template>
    </xsl:element>
  </xsl:template>

  <xsl:template match="display" mode="display_row">
    <xsl:param name="row" />

    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str" select="@template" />
      <xsl:with-param name="row" select="$row" />
    </xsl:call-template>
  </xsl:template>





<!-- -->
<!-- -->
<!-- -->
<!-- -->
<!-- -->
<!-- -->






  

  <!-- <xsl:template match="field[@type='linked']" mode="construct_linked_field"> -->
  <!--   <xsl:param name="data" /> -->

  <!--   <xsl:apply-templates select="." mode="construct_links"> -->
  <!--     <xsl:with-param name="row" select="$data" /> -->
  <!--   </xsl:apply-templates> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template match="field[@type='linked'][display/@style='table']" mode="construct_linked_field"> -->
  <!--   <xsl:param name="data" /> -->

  <!--   <div class="SFW_Host"> -->
  <!--     <xsl:element name="table"> -->
  <!--       <xsl:attribute name="class">Schema</xsl:attribute> -->
  <!--       <xsl:attribute name="data-sfw-class">lookup</xsl:attribute> -->
  <!--       <xsl:attribute name="data-sfw-input">input</xsl:attribute> -->
  <!--       <xsl:apply-templates select="display/@*" mode="add_on_click_attribute" /> -->
  <!--       <xsl:apply-templates select="ancestor::*[@rndx]" mode="add_result_attribute" /> -->
  <!--       <tbody> -->
  <!--         <xsl:apply-templates select="." mode="construct_links"> -->
  <!--           <xsl:with-param name="row" select="$data" /> -->
  <!--         </xsl:apply-templates> -->
  <!--       </tbody> -->
  <!--     </xsl:element> -->
  <!--   </div> -->
  <!-- </xsl:template> -->
  
  <!-- <xsl:template match="field[@type='linked'][not(child::display)]" mode="construct_linked_field"> -->
  <!--    <span>Linked field without link instructions (<xsl:value-of select="@name" />).  Count=<xsl:value-of select="local-name(*)" /></span> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template match="field[@type='linked']" mode="construct_links"> -->
  <!--   <xsl:param name="row" /> -->

  <!--   <xsl:variable name="str" select="$row/@*[local-name()=current()/@name]" /> -->
  <!--   <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" /> -->

  <!--   <xsl:apply-templates select="$result" mode="get_result_rows"> -->
  <!--     <xsl:with-param name="str" select="str" /> -->
  <!--     <xsl:with-param name="display" select="display" /> -->
  <!--   </xsl:apply-templates> -->

  <!-- </xsl:template> -->

  <!-- <xsl:template match="field[@type='linked'][@virtual]" mode="construct_links"> -->
  <!--   <xsl:param name="row" /> -->

  <!--   <xsl:variable name="idname"> -->
  <!--     <xsl:apply-templates select="$row/parent::*[@rndx]" mode="get_idname" /> -->
  <!--   </xsl:variable> -->
  <!--   <xsl:variable name="idval" select="$row/@*[local-name()=$idname]" /> -->

  <!--   <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" /> -->

  <!--   <xsl:apply-templates select="$result" mode="get_result_rows"> -->
  <!--     <xsl:with-param name="str" select="$idval" /> -->
  <!--     <xsl:with-param name="display" select="display" /> -->
  <!--   </xsl:apply-templates> -->

  <!-- </xsl:template> -->

  <!-- <xsl:template match="field[@type='linked'][not(@result)]" mode="construct_links"> -->
  <!--   <span>Linked field without a target result.</span> -->
  <!-- </xsl:template> -->

    

</xsl:stylesheet>
