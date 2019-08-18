<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!-- The elements in this stylesheet (should) have no external dependencies.
       Ensure that any additions herein meet that criteria.
  -->

  <xsl:variable name="nl"><xsl:text>
</xsl:text></xsl:variable>


  <xsl:variable name="quot">"</xsl:variable>
  <xsl:variable name="apos">'</xsl:variable>

  <!-- apostrophe,comma,apostrophe -->
  <xsl:variable name="aposcomapos">','</xsl:variable>
  <!-- apostrophe, apostrophe -->
  <xsl:variable name="apospair">''</xsl:variable>
  <!-- apostrophe AKA: alias for apostrophe for later replacement -->
  <xsl:variable name="aposaka">&#xfffd;</xsl:variable>

  <!-- for changing case -->
  <xsl:variable name="lowers">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="uppers">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

  <!-- Within strings that are delimited with apostrophes, a double apostrophe     -->
  <!-- often represents a single apostrophe in the represented string.  This       -->
  <!-- convention is widely used in CSV strings and database output.               -->
  
  <!-- The template translate_apospairs creates a string where double apostrophes  -->
  <!-- are converted into an untypeable alias character (DELETE (ascii 127) named  -->
  <!-- $aposaka.  The resulting string can use translate() to replace the $aposaka -->
  <!-- with a single apostrophe after the individual string has been extracted.    -->
  
  <xsl:template name="translate_apospairs">
    <!-- sfw_utilities.xsl -->
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

  <!-- Returns a path string for use in returning to the context element. -->
  <xsl:template match="*" mode="get_path">
    <!-- sfw_utilities.xsl -->
    <xsl:if test="parent::*">
      <xsl:apply-templates select="parent::*" mode="get_path" />
    </xsl:if>
    <xsl:value-of select="concat('/', name())" />
    <xsl:if test="name()='result'">
      <xsl:value-of select="concat('[@rndx=', @rndx, ']')" />
    </xsl:if>
  </xsl:template>

  <!-- Gets next element of a string that contains several items separated by '-' -->
  <xsl:template name="next_gid">
    <!-- sfw_utilities.xsl -->
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

  <!--
  The following variable and three templates will output a hierarchical
  view of the node or element selected with mode="dump".  Call it from
  within a <pre> element.

  To customize the spacer-span, simply define it in the stylesheet that
  imports sfw_generics.xsl (this stylesheet) and the spacer-span variable
  in the importing stylesheet will take precedence.

  A possible useful alternative spacer-span value might be '.  ' to more
  clearly indicate the indentation level.

  The process could be modified to translate certain entities that are not
  allowed in a pre element, or to output JSON.  I'm leaving that for later
  if needed.
  -->
  <xsl:variable name="spacer-span" select="'  '" />

  <xsl:template name="spacer">
    <xsl:param name="num" />

    <xsl:if test="$num &gt; 0">
      <xsl:value-of select="$spacer-span" />
      <xsl:call-template name="spacer">
        <xsl:with-param name="num" select="($num)-1" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@*" mode="dump">
    <xsl:param name="level" select="0" />
    
    <xsl:variable name="spaces">
      <xsl:call-template name="spacer">
        <xsl:with-param name="num" select="($level)" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="concat($spaces,local-name(),': ', .,$nl)" />

  </xsl:template>

  <xsl:template match="*" mode="dump">
    <xsl:param name="level" select="0" />
    
    <xsl:variable name="spaces">
      <xsl:call-template name="spacer">
        <xsl:with-param name="num" select="($level)" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="concat($nl,$spaces, local-name(), $nl)" />
    
    <xsl:apply-templates select="@*" mode="dump">
      <xsl:with-param name="level" select="($level)+1" />
    </xsl:apply-templates>
    
    <xsl:apply-templates select="*" mode="dump">
      <xsl:with-param name="level" select="($level)+1" />
    </xsl:apply-templates>
    
  </xsl:template>
  
</xsl:stylesheet>

