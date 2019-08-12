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

    <div class="{$class}" data-sfw-class="selectx" data-style="{$style}" data-sfw-control="true">
      <input type="hidden" name="{$name}" value="{$dval}" />
      <form onsubmit="return false;">
        <div class="display">
          <xsl:apply-templates select="." mode="fill_selectx_display">
            <xsl:with-param name="dval" select="$dval" />
          </xsl:apply-templates>
        </div>
        <input type="text" name="entry" class="entry" tabindex="0" />
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
      <xsl:param name="field" />
      <xsl:param name="show_name" />

      <xsl:element name="span">
        <xsl:if test="string-length($id_name)">
          <xsl:attribute name="data-id">
            <xsl:value-of select="@*[local-name()=$id_name]" />
          </xsl:attribute>
          <xsl:attribute name="title">Click to remove</xsl:attribute>
        </xsl:if>
        <xsl:apply-templates select="$field" mode="generate_field_value">
          <xsl:with-param name="row" select="." />
          <xsl:with-param name="show_name" select="$show_name" />
        </xsl:apply-templates>
      </xsl:element>

    </xsl:template>

    <xsl:template match="field[@type='selectx']" mode="fill_selectx_display">
      <xsl:param name="dval" />

      <xsl:variable name="style" select="@style" />

      <xsl:variable name="ids" select="concat(',',$dval,',')" />
      <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
      <xsl:variable name="field" select="." />

      <xsl:variable name="id_name">
        <xsl:apply-templates select="." mode="get_id_field" />
      </xsl:variable>

      <xsl:variable name="show_name">
        <xsl:apply-templates select="self::*[not(@display)]" mode="get_show_field" />
      </xsl:variable>

      <!-- Make node-list containing all rows whose id values are included in dval -->
      <xsl:variable
          name="on"
          select="$result/*[local-name()=../@row-name][contains($ids,concat(',',@*[local-name()=$id_name],','))]" />

      <div class="tabtarget" tabindex="0" />

      <xsl:for-each select="$on">
        <xsl:apply-templates select="." mode="build_selectx_span">
          <xsl:with-param name="id_name">
            <xsl:if test="@style='multiple'">
              <xsl:value-of select="$id_name" />
            </xsl:if>
          </xsl:with-param>
          <xsl:with-param name="field" select="$field" />
          <xsl:with-param name="show_name" select="$show_name" />
        </xsl:apply-templates>
        
        <xsl:if test="position() != last()">, </xsl:if>
      </xsl:for-each>

    </xsl:template>

    <xsl:template match="field[@type='selectx'][@ranked]" mode="fill_selectx_display">
      <xsl:param name="dval" />

      <div class="tabtarget" tabindex="0" />

      <xsl:apply-templates select="." mode="fill_selectx_display_lis">
        <xsl:with-param name="dval" select="$dval" />
      </xsl:apply-templates>
    </xsl:template>

    <xsl:template match="field[@type='selectx'][@ranked]" mode="fill_selectx_display_lis">
      <xsl:param name="dval" />

      <xsl:variable name="id_name">
        <xsl:apply-templates select="." mode="get_id_field" />
      </xsl:variable>

      <xsl:variable name="show_name">
        <xsl:apply-templates select="self::*[not(@display)]" mode="get_show_field" />
      </xsl:variable>

      <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
      <xsl:variable name="field" select="." />

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
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="show_name" select="$show_name" />
      </xsl:apply-templates>
        
      <xsl:if test="$b_len">
        <xsl:text>, </xsl:text>
        <xsl:apply-templates select="." mode="fill_selectx_display_lis">
          <xsl:with-param name="dval" select="substring($dval,($b_len+2))" />
        </xsl:apply-templates>
      </xsl:if>

    </xsl:template>

    <xsl:template match="field[@type='selectx']" mode="fill_selectx_ul">
      <xsl:param name="dval" />

      <xsl:variable name="ids" select="concat(',',$dval,',')" />
      <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]" />
      <xsl:variable name="field" select="." />

      <xsl:variable name="id_name">
        <xsl:apply-templates select="." mode="get_id_field" />
      </xsl:variable>

      <xsl:variable name="show_name">
        <xsl:apply-templates select="self::*[not(@display)]" mode="get_show_field" />
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
          <xsl:apply-templates select="$field" mode="generate_field_value">
            <xsl:with-param name="row" select="." />
            <xsl:with-param name="show_name" select="$show_name" />
          </xsl:apply-templates>
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
      <xsl:variable name="result" select="/*/*[local-name()=current()/@result]" />
      <xsl:if test="$result">
        <xsl:choose>
          <xsl:when test="@id_field"><xsl:value-of select="@id_field" /></xsl:when>
          <xsl:when test="$result/@id_field"><xsl:value-of select="$result/@id_field" /></xsl:when>
          <xsl:when test="$result/@lookup"><xsl:value-of select="$result/@lookup" /></xsl:when>
          <xsl:when test="$result/schema">
            <xsl:variable name="fcount" select="count($result/schema/field)" />
            <xsl:choose>
              <xsl:when test="$result/schema/field[@primary-key][1]">
                <xsl:value-of select="$result/schema/field[@primary-key][1]/@name" />
              </xsl:when>
              <xsl:otherwise>
              <xsl:value-of select="$result/schema/field[1]/@name" />
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          <xsl:otherwise>
            <xsl:variable
                name="row1"
                select="/*/*[local-name()=current()/@result][@rndx]/*[local-name()=../@row-name]" />
            <xsl:if test="$row1">
              <xsl:value-of select="local-name($row1/@*[1])" />
            </xsl:if>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:template>

    <xsl:template match="field[@result]" mode="get_show_field">
      <xsl:variable name="result" select="/*/*[local-name()=current()/@result][@rndx]" />
      <xsl:if test="$result">
        <xsl:choose>
          <xsl:when test="@show"><xsl:value-of select="@show" /></xsl:when>
          <xsl:when test="$result/@show"><xsl:value-of select="$result/@show" /></xsl:when>
          <xsl:otherwise>
            <xsl:variable name="schema" select="$result/schema" />
            <xsl:variable name="row1" select="$result/*[local-name()=../@row-name][1]" />
            <xsl:choose>
              <xsl:when test="$schema">
                <xsl:variable name="fcount" select="count($schema/field)" />
                <xsl:choose>
                  <xsl:when test="$fcount = 1">
                    <xsl:value-of select="$schema/field[1]/@name" />
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:variable name="id_field">
                      <xsl:apply-templates select="." mode="get_id_field" />
                    </xsl:variable>
                    <xsl:value-of select="$schema/field[not(@name=$id_field)]/@name" />
                  </xsl:otherwise>
                </xsl:choose>
              </xsl:when>
              <xsl:when test="$row1">
                <xsl:variable name="rac" select="count($row1/@*)" />
                <xsl:choose>
                  <xsl:when test="$rac &gt; 1">
                    <xsl:value-of select="local-name($row1/@*[2])" />
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:value-of select="local-name($row1/@*[1])" />
                  </xsl:otherwise>
                </xsl:choose>
              </xsl:when>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:template>

    <xsl:template match="field" mode="get_indicies_from_data">
      <xsl:param name="data" />
      <xsl:value-of select="concat(',',$data/@*[local-name()=current()/@name],',')" />
    </xsl:template>

</xsl:stylesheet>
