<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!--
  This stylesheet contains the form-constructing templates for
  creating a group element.  It's an artifact of an earlier project.

  A group element is a table embedded in a form.  The one implementation
  of this element type was to have a family form with a group element
  containing the members of the family.  The contents of the embedded
  table could be editted in place, and the composite value would be
  packed into a single string and unpacked on the server to update
  other database tables.

  This is not supported in the SchemaFW, and probably will never be
  supported because it's so much easier to open a subordinate window
  to manage what the group did in-form.  The code necessary to support
  this feature, in Javascript and C++ for a MySQL external function,
  have  been abandoned.  The templates are separated out and kept around
  in case I missed some dependencies.
  -->

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

</xsl:stylesheet>
