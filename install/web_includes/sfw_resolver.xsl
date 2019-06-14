<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_uuescape.xsl" />
  
  <!--
      This template produces a string from the "str" param with tokens
      replaced with values included in document.
  -->
  <xsl:template name="resolve_refs">
    <xsl:param name="str" />
    <xsl:param name="row" select="/.." />
    <xsl:param name="enclose" />
    <xsl:param name="escape" />

    <!-- The 'delim' is the character just after the { that
         indicates how the following work is interpreted. -->
    <xsl:variable name="delim">
      <xsl:variable name="after" select="substring(substring-after($str,'{'),1,1)" />
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

    <xsl:variable name="raw_ref">
      <xsl:if test="starts-with($str,$delim)">
        <xsl:variable name="end" select="substring-before($str,'}')" />
        <xsl:if test="$end and not (contains($end,' '))">
          <xsl:value-of select="substring($end, 3)" />
        </xsl:if>
      </xsl:if>
    </xsl:variable>

    <xsl:variable name="ref_hints" select="substring-after($raw_ref,';')" />
    <xsl:variable name="b_ref" select="substring-before($raw_ref,';')" />
    <xsl:variable name="s_ref" select="substring($raw_ref,1 div boolean(string-length($b_ref)=0))" />
    <xsl:variable name="ref" select="concat($b_ref, $s_ref)" />

    <xsl:variable name="pval">
      <xsl:choose>
        <xsl:when test="$len_before">
          <xsl:value-of select="$before" />
        </xsl:when>
        <xsl:when test="string-length($ref)">
          <xsl:variable name="type" select="substring($str,1,2)" />
          <xsl:choose>
            <xsl:when test="$type='{$'">
              <xsl:call-template name="get_var_value">
                <xsl:with-param name="name" select="$ref" />
              </xsl:call-template>
            </xsl:when>
            <xsl:when test="$type='{@'">
              <xsl:call-template name="get_data_value">
                <xsl:with-param name="name" select="$ref" />
              </xsl:call-template>
            </xsl:when>
            <xsl:when test="$type='{!'">
              <xsl:choose>
                <xsl:when test="$row">
                  <xsl:apply-templates select="$row" mode="get_value_from_row">
                    <xsl:with-param name="name" select="$ref" />
                  </xsl:apply-templates>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:value-of select="concat($delim, $ref, '}')" />
                </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
          </xsl:choose>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$str" /></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="val">
      <xsl:choose>
        <!-- if in escape mode, replace references will be URL-escaped -->
        <xsl:when test="$escape='1' and string-length($raw_ref) and contains('$@',substring($delim,2,1))">
          <xsl:call-template name="uuescape">
            <xsl:with-param name="str" select="$pval" />
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$pval" /></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="after">
      <xsl:choose>
        <xsl:when test="string-length($raw_ref) &gt; 0">
          <xsl:value-of select="substring($str,string-length($raw_ref)+4)" />
        </xsl:when>
        <xsl:when test="$len_before &gt; 0">
          <xsl:value-of select="substring($str,($len_before)+1)" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <!-- Preserve and compress, if necessary, spaces on either end of the string,
         by surrounding the string with #s and removing them afterwards: -->
    <xsl:variable name="trimmed">
      <xsl:variable name="extra" select="normalize-space(concat('#',$val,'#'))" />
      <xsl:value-of select="substring($extra,2,string-length($extra) - 2)" />
    </xsl:variable>

    <xsl:if test="string-length($trimmed) &gt; 0">
      <xsl:choose>
        <xsl:when test="$enclose">
          <xsl:call-template name="enclose_val">
            <xsl:with-param name="str" select="$trimmed" />
            <xsl:with-param name="tag" select="$enclose" />
            <xsl:with-param name="hints" select="$ref_hints" />
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$trimmed" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <xsl:if test="string-length($after) &gt; 0">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="$after" />
        <xsl:with-param name="row" select="$row" />
        <xsl:with-param name="enclose" select="$enclose" />
        <xsl:with-param name="escape" select="$escape" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@*" mode="resolve_refs">
    <xsl:param name="escape" />
    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str" select="." />
      <xsl:with-param name="escape" select="$escape" />
    </xsl:call-template>
  </xsl:template>

  <!-- For resolving a @ reference, named attribute in a form-type result. -->
  <xsl:template name="get_data_value">
    <xsl:param name="name" />
    <xsl:variable name="results" select="/*/*[@rndx and not(@type='variables')]" />

    <xsl:variable name="formrs" 
                  select="$results[count(*[local-name()=../@row-name])=1]" />
    <xsl:variable name="rows" select="$formrs/*[local-name()=../@row-name]" />
    <xsl:variable name="attrs" select="$rows/@*[local-name()=$name]" />

    <xsl:if test="count($attrs)">
      <xsl:value-of select="$attrs[1]" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="get_var_value">
    <xsl:param name="name" />
    <xsl:variable name="row" select="*[local-name()=current()/@row-name]" />
    <xsl:if test="$row">
      <xsl:variable name="val" select="$row[1]/@*[local-name()=$name]" />
      <xsl:if test="$val">
        <xsl:value-of select="$val" />
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template name="get_var_value">
    <xsl:param name="name" />

    <xsl:variable name="prime">
      <xsl:apply-templates select="$vars" mode="get_var_value">
        <xsl:with-param name="name" select="$name" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="string-length($prime)">
        <xsl:value-of select="$prime" />
      </xsl:when>
      <xsl:when test="count($alt_vars) &gt; 0">
        <xsl:apply-templates select="$alt_vars" mode="get_var_value">
          <xsl:with-param name="name" select="$name" />
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="get_value_from_row">
    <xsl:param name="name" />
    <xsl:value-of select="current()/@*[local-name()=$name]" />
  </xsl:template>

  <xsl:template name="add_enclose_attributes">
    <xsl:param name="hints" />
    <xsl:variable name="c_hint" select="substring-before($hints,'|')" />
    <xsl:variable name="s_hint" select="substring($hints,1 div boolean(string-length($c_hint)=0))" />
    <xsl:variable name="hint" select="concat($c_hint,$s_hint)" />

    <xsl:variable name="name" select="normalize-space(substring-before($hint,'='))" />
    <xsl:variable name="val" select="substring-after($hint,'=')" />

    <xsl:attribute name="{$name}">
      <xsl:value-of select="$val" />
    </xsl:attribute>

    <xsl:if test="string-length($c_hint)&gt;0">
      <xsl:call-template name="add_enclose_attributes">
        <xsl:with-param name="hints" select="substring-after($hints,'|')" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="enclose_val">
    <xsl:param name="str" />
    <xsl:param name="tag" />
    <xsl:param name="hints" />

    <xsl:element name="{$tag}">
      <xsl:if test="string-length($hints)&gt;0">
        <xsl:call-template name="add_enclose_attributes">
          <xsl:with-param name="hints" select="$hints" />
        </xsl:call-template>
      </xsl:if>
      <xsl:value-of select="$str" />
    </xsl:element>
  </xsl:template>

  
</xsl:stylesheet>
