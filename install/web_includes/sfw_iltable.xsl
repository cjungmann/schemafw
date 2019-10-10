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

  <!-- Modeless attribute match for like-named field of proper type in hosting schema. -->
  <xsl:template match="@*[local-name()=(ancestor::schema/field[@type='iltable']/@name)]">

    <xsl:variable name="field" select="ancestor::schema/field[@name=local-name(current())]" />
    <xsl:variable name="result" select="/*/*[local-name()=$field/@result][@rndx]" />

    <xsl:choose>
      <xsl:when test="not($result)">
        <div>Failed to find required result for itable instruction.</div>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="transform_associated_references">
          <xsl:with-param name="result" select="$result" />
          <xsl:with-param name="field" select="$field" />
          <xsl:with-param name="str" select="." />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Template that is called by construct_form via
       construct_input_row via
       construct_form_input via
       construct_form
  -->
  <xsl:template match="schema/field[@type='iltable'][@result]" mode="construct_input">
    <xsl:param name="data" />

    <xsl:variable name="jresult" select="/*/*[local-name()=current()/@result]" />
    <xsl:variable name="schema" select="$jresult/schema" />

    <div class="SFW_Host" style="position:static">
      <xsl:element name="table">
        <xsl:attribute name="class">Schema</xsl:attribute>
        <xsl:apply-templates select="$schema" mode="add_result_attribute" />
        <xsl:apply-templates select="$schema" mode="add_sfw_class_attribute">
          <xsl:with-param name="sfw_class" select="'iltable'" />
        </xsl:apply-templates>
        <xsl:apply-templates select="$schema" mode="add_on_click_attributes" />

        <thead>
          <xsl:apply-templates select="." mode="make_add_button" />
          <xsl:apply-templates select="$schema" mode="construct_thead_rows" />
        </thead>

        <tbody>
          <xsl:apply-templates select="$schema" mode="fill_tbody" />
        </tbody>

      </xsl:element>
    </div>

  </xsl:template>

  <xsl:template match="schema/field[@type='iltable']" mode="make_add_button"></xsl:template>
  <xsl:template match="schema/field[@type='iltable'][@on_add]" mode="make_add_button">
    <tr>
      <td colspan="99">
        <button type="button" name="add">+</button>
      </td>
    </tr>
  </xsl:template>

  <xsl:template match="schema/field[@type='iltable'][@result]" mode="display_value">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="construct_input">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <!--
      We gotta figure out a way to reuse the person[parent::people] element in
      different contexts.  For example, one context may be just listing the names
      in a single string, and another view might be in a table with more than one
      cell for different attributes.
  -->
  <xsl:template match="person[parent::people]" mode="transform_row">
    <xsl:param name="field" />
    
  </xsl:template>



</xsl:stylesheet>
