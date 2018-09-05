<?xml version="1.0" encoding="utf-8" ?>

<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
>
   <!-- exclude-result-prefixes="html" -->
   <!-- xmlns="http://www.w3.org/1999/xhtml" -->
   <!-- xmlns:html="http://www.w3.org/1999/xhtml" -->

  <xsl:variable name="xdigs">0123456789ABCDEF</xsl:variable>

  <xsl:variable       name="uuasciimap"> !&quot;#$%&amp;'()*+,-./0123456789:;&lt;=&gt;?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~</xsl:variable>

  <xsl:variable name="uureplaceablemap">  &quot;#$%&amp;    +,  /          :;&lt;=&gt;?@                          [\]^ `                          {|}~</xsl:variable>

  <xsl:variable name="uu8859map">&#160;&#161;&#162;&#163;&#164;&#165;&#166;&#167;&#168;&#169;&#170;&#171;&#172;&#173;&#174;&#175;&#176;&#177;&#178;&#179;&#180;&#181;&#182;&#183;&#184;&#185;&#186;&#187;&#188;&#189;&#190;&#191;&#192;&#193;&#194;&#195;&#196;&#197;&#198;&#199;&#200;&#201;&#202;&#203;&#204;&#205;&#206;&#207;&#208;&#209;&#210;&#211;&#212;&#213;&#214;&#215;&#216;&#217;&#218;&#219;&#220;&#221;&#222;&#223;&#224;&#225;&#226;&#227;&#228;&#229;&#230;&#231;&#232;&#233;&#234;&#235;&#236;&#237;&#238;&#239;&#240;&#241;&#242;&#243;&#244;&#245;&#246;&#247;&#248;&#249;&#250;&#251;&#252;&#253;&#254;&#255;</xsl:variable>

  <!-- includes characters that must be escaped, spaces where not required: -->
  <xsl:variable name="uemap" select="concat($uureplaceablemap, $uu8859map)" />

  <!-- Two strings, for translate(), to detect if string includes unhandled character (out-of-range: -->
  <xsl:variable name="space10" select="'          '" />
  <xsl:variable name="space20" select="concat($space10,$space10)" />
  <xsl:variable name="space50" select="concat($space20,$space20,$space10)" />
  <xsl:variable name="uuallowedmap" select="concat($space50,$space50,$space50,$space20,$space20,' ')" />
  <xsl:variable name="uuallowed" select="concat($uuasciimap,$uu8859map)" />

  <xsl:template name="check_range">
    <xsl:param name="str" />
    <xsl:variable name="test" select="normalize-space(translate($str,$uuallowedmap, $uuallowed))" />
    <check_range test="{$test}" />
    <xsl:if test="string-length($test)">
      <xsl:message>Unexpected characters in string to be escaped.</xsl:message>
    </xsl:if>
  </xsl:template>


  <xsl:template name="hexify">
    <xsl:param name="val" />
    <xsl:variable name="lsn" select="$val mod 16" />
    <xsl:variable name="msn" select="floor(($val) div 16)" />

    <xsl:text>%</xsl:text>
    <xsl:value-of select="substring($xdigs,($msn)+1,1)" />
    <xsl:value-of select="substring($xdigs,($lsn)+1,1)" />
  </xsl:template>

  <xsl:template name="uuescape">
    <xsl:param name="str" />
    <xsl:variable name="char" select="substring($str,1,1)" />
    <xsl:variable name="uubefore" select="substring-before($uemap,$char)" />
    <xsl:variable name="uublen" select="string-length($uubefore)" />

    <xsl:choose>
      <xsl:when test="$uublen &gt; 0">
        <xsl:call-template name="hexify">
          <xsl:with-param name="val">
            <xsl:choose>
              <xsl:when test="$uublen &gt; 94">
                <xsl:value-of select="($uublen)+65" />
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="($uublen)+32" />
              </xsl:otherwise>
            </xsl:choose>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="$char=' '">%20</xsl:when>
      <xsl:otherwise><xsl:value-of select="$char" /></xsl:otherwise>
    </xsl:choose>

    <xsl:if test="string-length($str) &gt; 1">
      <xsl:call-template name="uuescape">
        <xsl:with-param name="str" select="substring($str,2)" />
      </xsl:call-template>
    </xsl:if>

  </xsl:template>



  <xsl:template match="/">
    <xsl:variable name="lenall" select="string-length($uuallowed)" />
    <xsl:variable name="lenmap" select="string-length($uuallowedmap)" />
    <xsl:variable name="apos">'</xsl:variable>
    <xsl:variable name="nl">
</xsl:variable>

    <xsl:if test="$lenall != $lenmap">
      <xsl:value-of select="concat('allowedlen=',$lenall,' != allowedmap=',$lenmap)" />
    </xsl:if>

    <xsl:apply-templates select="*" />
  </xsl:template>

  <xsl:template match="test">
    
    <xsl:text>
    </xsl:text>
    <!-- <xsl:call-template name="check_range"> -->
    <!--   <xsl:with-param name="str" select="." /> -->
    <!-- </xsl:call-template> -->

    <xsl:element name="test">
      <xsl:attribute name="source"><xsl:value-of select="." /></xsl:attribute>
      <xsl:attribute name="result">
        <xsl:call-template name="uuescape">
          <xsl:with-param name="str" select="." />
        </xsl:call-template>
      </xsl:attribute>
    </xsl:element>

  </xsl:template>

</xsl:stylesheet>
