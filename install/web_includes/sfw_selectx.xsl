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


  <xsl:template match="field[@type='selectx']" mode="get_value">
    <xsl:param name="data" />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <xsl:if test="count($result)=0">
      <xsl:apply-templates select="." mode="missing_result" />
    </xsl:if>

    <xsl:apply-templates select="." mode="fill_selectx_display">
      <xsl:with-param name="dval" select="$data/@*[local-name()=current()/@name]" />
    </xsl:apply-templates>
  </xsl:template>


  <xsl:template match="field[@type='selectx']" mode="construct_input">
    <xsl:param name="data" />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
    <xsl:if test="count($result)=0">
      <xsl:apply-templates select="." mode="missing_result" />
    </xsl:if>
    
    <xsl:variable name="dval" select="$data/@*[local-name()=current()/@name]" />

    <xsl:variable name="name">
      <xsl:apply-templates select="." mode="get_name" />
    </xsl:variable>

    <xsl:variable name="style">
      <xsl:choose>
        <xsl:when test="@style"><xsl:value-of select="@style" /></xsl:when>
        <xsl:otherwise>single</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="class">
      <xsl:text>selectx</xsl:text>
      <xsl:if test="$style!='single'">
         <xsl:value-of select="concat(' ',$style)" />
      </xsl:if>
    </xsl:variable>

    <div class="{$class}" data-sfw-class="selectx" data-style="{$style}" data-sfw-input="true">
      <input type="hidden" name="{$name}" value="{$dval}" />
      <form onsubmit="return false;">
        <div class="display" tabindex="0">
          <xsl:apply-templates select="." mode="fill_selectx_display">
            <xsl:with-param name="dval" select="$dval" />
          </xsl:apply-templates>
        </div>
        <input type="text" name="entry" class="entry" />
        <ul>
          <xsl:apply-templates select="." mode="fill_selectx_ul">
            <xsl:with-param name="dval" select="$dval" />
          </xsl:apply-templates>
        </ul>
      </form>
    </div>

    </xsl:template>


    <!-- Error message templates for unqualified field matches -->
    <xsl:template match="field[@type='selectx'][not(@result)]" mode="get_value">
      <xsl:apply-templates select="." mode="missing_result" />
    </xsl:template>

    <xsl:template match="field[@type='selectx'][not(@result)]" mode="construct_input">
      <xsl:apply-templates select="." mode="missing_result" />
    </xsl:template>

    <!-- selectx-specific templates -->

    <xsl:template match="*" mode="build_selectx_span">
      <xsl:param name="id_name" />
      <xsl:param name="show_name" />

      <!-- <span> -->
      <!--   <xsl:text>|</xsl:text> -->
      <!--   <xsl:value-of select="concat('id=',@*[local-name()=$id_name])" /> -->
      <!--   <xsl:text> </xsl:text> -->
      <!--   <xsl:value-of select="concat('value=',@*[local-name()=$show_name])" /> -->
      <!--   <xsl:text>|</xsl:text> -->
      <!-- </span> -->

      <xsl:element name="span">
        <xsl:if test="string-length($id_name)">
          <xsl:attribute name="data-id">
            <xsl:value-of select="@*[local-name()=$id_name]" />
          </xsl:attribute>
          <xsl:attribute name="title">Click to remove</xsl:attribute>
        </xsl:if>
        <xsl:value-of select="@*[local-name()=$show_name]" />
      </xsl:element>

    </xsl:template>

    <xsl:template match="field[@type='selectx'][@for-each]" mode="fill_selectx_display">
      <xsl:param name="dval" />

      <xsl:variable name="style" select="@style" />

      <xsl:variable name="ids" select="concat(',',$dval,',')" />
      <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />

      <xsl:variable name="id_name">
        <xsl:apply-templates select="." mode="get_id_field" />
      </xsl:variable>

      <xsl:variable name="show_name">
        <xsl:apply-templates select="." mode="get_show_field" />
      </xsl:variable>

      <!-- Make node-list containing all rows whose id values are included in dval -->
      <xsl:variable
          name="on"
          select="$result/*[local-name()=../@row-name][contains($ids,concat(',',@*[local-name()=$id_name],','))]" />

      <xsl:for-each select="$on">
        
        <xsl:apply-templates select="." mode="build_selectx_span">
          <xsl:with-param name="id_name">
            <xsl:if test="@style='multiple'">
              <xsl:value-of select="$id_name" />
            </xsl:if>
          </xsl:with-param>
          <xsl:with-param name="show_name" select="$show_name" />
        </xsl:apply-templates>
        
        <xsl:if test="position() != last()">, </xsl:if>
      </xsl:for-each>

    </xsl:template>

    <xsl:template match="field[@type='selectx']" mode="fill_selectx_display">
      <xsl:param name="dval" />

      <xsl:variable name="id_name">
        <xsl:apply-templates select="." mode="get_id_field" />
      </xsl:variable>

      <xsl:variable name="show_name">
        <xsl:apply-templates select="." mode="get_show_field" />
      </xsl:variable>

      <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />

      <xsl:variable name="b_id" select="substring-before($dval,',')" />
      <xsl:variable name="b_len" select="string-length($b_id)" />
      <xsl:variable name="s_id"
          select="substring($dval,1 div boolean($b_len=0))" />

      <xsl:variable name="id" select="concat($b_id, $s_id)" />

      <xsl:variable
          name="row"
          select="$result/*[local-name()=../@row-name][@*[local-name()=$id_name]=$id]" />

      <xsl:apply-templates select="$row" mode="build_selectx_span">
        <xsl:with-param name="id_name">
          <xsl:if test="@style='multiple'">
            <xsl:value-of select="$id_name" />
          </xsl:if>
        </xsl:with-param>
        <xsl:with-param name="show_name" select="$show_name" />
      </xsl:apply-templates>
        
      <xsl:if test="$b_len">
        <xsl:text>, </xsl:text>
        <xsl:apply-templates select="." mode="fill_selectx_display">
          <xsl:with-param name="dval" select="substring($dval,($b_len+2))" />
        </xsl:apply-templates>
      </xsl:if>

    </xsl:template>

    <xsl:template match="field[@type='selectx']" mode="fill_selectx_ul">
      <xsl:param name="dval" />

      <xsl:variable name="ids" select="concat(',',$dval,',')" />
      <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />

      <xsl:variable name="id_name">
        <xsl:apply-templates select="." mode="get_id_field" />
      </xsl:variable>

      <xsl:variable name="show_name">
        <xsl:apply-templates select="." mode="get_show_field" />
      </xsl:variable>

      <xsl:for-each select="$result/*[local-name()=../@row-name]">
        <xsl:variable name="idval" select="@*[local-name()=$id_name]" />
        <xsl:variable name="idvalc" select="concat(',',$idval,',')" />

        <xsl:element name="li">
          <xsl:attribute name="data-value">
            <xsl:value-of select="$idval" />
          </xsl:attribute>
          <xsl:if test="contains($ids,concat(',',$idval,','))">
            <xsl:attribute name="class">on</xsl:attribute>
          </xsl:if>
          <xsl:value-of select="@*[local-name()=$show_name]" />
        </xsl:element>
      </xsl:for-each>
    </xsl:template>

    <!-- Modeless templates to be called from Javascript -->

    <xsl:template match="field[@type='selectx'][@selectx_display]">
      <xsl:apply-templates select="." mode="fill_selectx_display">
        <xsl:with-param name="dval" select="@selectx_display" />
      </xsl:apply-templates>
    </xsl:template>

    <xsl:template match="field[@type='selectx'][@selectx_ul]">
      <xsl:apply-templates select="." mode="fill_selectx_ul">
        <xsl:with-param name="dval" select="@selectx_ul" />
      </xsl:apply-templates>
    </xsl:template>

    <!-- END OF selectx-specific templates -->



    <!-- The following templates should migrate to sfw_schema.xsl after they've been established. -->
    <xsl:template match="field[@result]" mode="missing_schema">
      <div>Failed to find a schema for field '<xsl:value-of select="@name" />'</div>
    </xsl:template>

    <xsl:template match="field" mode="missing_result">
      <div>Failed to find a result for field '<xsl:value-of select="@name" />'</div>
    </xsl:template>

    <xsl:template match="field[@result]" mode="get_id_field">
      <xsl:variable name="schema"
                    select="/*/*[@rndx][local-name()=current()/@result]/schema" />

      <xsl:if test="not($schema)">
        <xsl:apply-templates select="." mode="missing_schema" />
      </xsl:if>

      <xsl:value-of select="$schema/field[@primary-key][1]/@name" />
      
    </xsl:template>

    <xsl:template match="field[@result]" mode="get_show_field">
      <xsl:variable name="schema"
                    select="/*/*[@rndx][local-name()=current()/@result]/schema" />
      <xsl:if test="not($schema)">
        <xsl:apply-templates select="." mode="missing_schema" />
      </xsl:if>

      <xsl:choose>
        <xsl:when test="@show"><xsl:value-of select="@show" /></xsl:when>
        <xsl:otherwise>
          <xsl:variable name="id_name">
            <xsl:apply-templates select="." mode="get_id_field" />
          </xsl:variable>
          <xsl:variable name="non_id" select="$schema/field[@name!=$id_name][1]/@name" />
          <xsl:choose>
            <xsl:when test="$non_id"><xsl:value-of select="$non_id" /></xsl:when>
            <xsl:otherwise><xsl:value-of select="$id_name" /></xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:template>

    <xsl:template match="field" mode="get_indicies_from_data">
      <xsl:param name="data" />
      <xsl:value-of select="concat(',',$data/@*[local-name()=current()/@name],',')" />
    </xsl:template>

</xsl:stylesheet>
