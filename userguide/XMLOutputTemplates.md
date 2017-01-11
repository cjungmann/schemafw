# XML Output Templates

Sometimes it is helpful to study the XML document that is the source of a
page or view.  The templates that follow the example can be pasted into your
_default.xsl_ file to append a representation of the XML to a page view.

_Except from the match="/" template in default.xsl_
~~~{xsl}
<div id="SFW_Content">
   <div class="SFW_Host">
   <xsl:apply-templates select="/*" mode="show_document_content" />

   <!-- The following application will display the XML document -->
   <xsl:apply-templates select="/*" mode="showit" />
  </div>
</div>
~~~

The following set of templates includes a beginning and ending comment to
help identify the code for removal when it's no longer required.  Cut and paste
the lines into your _default.xsl_ file.

~~~{xsl}
  <!-- Start of SHOWIT templates -->
  <xsl:template name="tabber">
    <xsl:param name="left" />
    <xsl:if test="$left and $left &gt; 0">
      <xsl:text>   </xsl:text>
      <xsl:call-template name="tabber">
        <xsl:with-param name="left" select="-1+$left" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@*" mode="showattr">
    <xsl:value-of select="concat(' ', local-name(), '=&quot;', ., '&quot;')" />
  </xsl:template>

  <xsl:template match="*" mode="showel">
    <xsl:param name="level" select="0" />

    <!-- Likely redundant, but included within the template to ensure availability -->
    <xsl:variable name="nl"><xsl:text>
</xsl:text></xsl:variable>
    
    <xsl:variable name="tag" select="local-name()" />

    <xsl:call-template name="tabber">
      <xsl:with-param name="left" select="$level" />
    </xsl:call-template>
    
    <xsl:value-of select="concat('&lt;', $tag)" />
    <xsl:apply-templates select="@*" mode="showattr" />

    <xsl:choose>
      <xsl:when test="count(*)">
        <xsl:value-of select="concat('&gt;',$nl)" />

        <xsl:apply-templates select="*" mode="showel">
          <xsl:with-param name="level" select="1+$level" />
        </xsl:apply-templates>

        <xsl:call-template name="tabber">
          <xsl:with-param name="left" select="$level" />
        </xsl:call-template>
        <xsl:value-of select="concat('&lt;/', $tag, '&gt;', $nl)" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="concat(' /&gt;',$nl)" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*" mode="showit">
     <pre><xsl:apply-templates select="." mode="showel" /></pre>
  </xsl:template>
  <!-- End of SHOWIT templates -->
~~~

