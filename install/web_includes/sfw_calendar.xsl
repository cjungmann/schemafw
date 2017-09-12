<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_generics.xsl" />
  <xsl:import href="sfw_schema.xsl" />
  
<!--
This stylesheet is intended to be imported into another stylesheet.

It expects to be passed an element with the following attributes:
   @month       YYYY-MM
   @initialDay  The day-of-week of the first of the month
   @countOfDays The number of days/last day of the month.
These are optional:
   @today       YYYY-MM-DD
   @selected    YYYY-MM-DD

The default is to print the calendar from Sunday to Saturday.
the $day_left and $day_right variables can squeeze the calendar.
Set $day-left=2 and $day-right=6 to make a Monday-Friday calendar.

Pass class name or names in the table_class param when you call the
build_calendar template.  Other classes that are automatically
set are:

every day td will include the cal_day class.
td.today for a date that matches @today
td.selected for a date that matches @selected
div.day_head for the number at the top of each day

Customize your calendar by creating a stylesheet to handle these
classes and by overriding the named-template build_day_content.
This template will be called with a single 'date' parameter which
will contain the YYYY-MM-DD date.
 -->

  <!--
      Change these variables by defining them in the stylesheet
      that imports this stylesheet.

      1 is Sunday, 7 is Saturday.  Display weekdays only by using
      day_left="2" day_right="6"
  -->
  <xsl:variable name="day_left" select="1" />
  <xsl:variable name="day_right" select="7" />

  <!-- The variables for day and month names precede templates that use them. -->
  <xsl:variable name="day_names"
                select="'Sunday   Monday   Tuesday  WednesdayThursday Friday   Saturday '" />
  <xsl:template name="get_day_name">
    <xsl:param name="ndx" />
    <xsl:value-of select="normalize-space(substring($day_names,1+((($ndx)-1)*9),9))" />
  </xsl:template>

  <xsl:variable name="month_names"
                select="'January  February March    April    May      June     July     August   SeptemberOctober  November December '" />

  <xsl:template name="get_month_name">
    <xsl:param name="ndx" />
    <xsl:value-of select="substring($month_names,1+((($ndx)-1)*9),9)" />
  </xsl:template>

  <xsl:template name="make_next_month">
    <xsl:param name="month" />
    <xsl:variable name="old_year" select="substring($month,1,4)" />
    <xsl:variable name="newmonth" select="((substring($month,6,2)) mod 12) + 1" />
    
    <xsl:variable name="year">
      <xsl:choose>
        <xsl:when test="$newmonth=1">
          <xsl:value-of select="($old_year)+1" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$old_year" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:value-of select="concat($year,'-',substring((($newmonth)+100),2))" />
  </xsl:template>

  <xsl:template name="make_day_heads">
    <xsl:param name="day" select="$day_left" />
    <xsl:variable name="day_name">
      <xsl:call-template name="get_day_name">
        <xsl:with-param name="ndx" select="$day" />
      </xsl:call-template>
    </xsl:variable>
    <td class="cal_day_head"><xsl:value-of select="substring($day_name,1,3)" /></td>
    
    <xsl:if test="($day)+1 &lt;=($day_right)">
      <xsl:call-template name="make_day_heads">
        <xsl:with-param name="day" select="($day)+1" />
      </xsl:call-template>
    </xsl:if>
    
  </xsl:template>

  <xsl:template name="build_day_content">
    <xsl:param name="date" />
  </xsl:template>

  <xsl:template name="build_day">
    <xsl:param name="today" />
    <xsl:param name="month" />
    <xsl:param name="lastday" />
    <xsl:param name="ndx" />
    <xsl:param name="date" />

    <xsl:if test="($ndx)&gt;=($day_left)">
      <xsl:variable name="ddate">
        <xsl:choose>
          <xsl:when test="$date &gt; $lastday">
            <xsl:value-of select="(($date) - ($lastday))" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$date" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="zdate" select="substring((($ddate)+100),2)" />

      <xsl:variable name="fulldate">
        <xsl:choose>
          <xsl:when test="$ddate != $date">
            <xsl:variable name="nextmonth">
              <xsl:call-template name="make_next_month">
                <xsl:with-param name="month" select="$month" />
              </xsl:call-template>
            </xsl:variable>
            <xsl:value-of select="concat($nextmonth,'-',$zdate)" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="concat($month,'-',$zdate)" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="classes">
        <xsl:text>cal_day</xsl:text>
        <xsl:if test="$date!=$ddate">
          <xsl:value-of select="' nextmonth'" />
        </xsl:if>
        <xsl:if test="$fulldate=$today">
          <xsl:value-of select="' today'" />
        </xsl:if>
      </xsl:variable>

      <xsl:element name="td">
        <xsl:if test="string-length($classes)">
          <xsl:attribute name="class">
            <xsl:value-of select="$classes" />
          </xsl:attribute>
        </xsl:if>
        <xsl:if test="$date&gt;0 and $date&lt;=$lastday">
          <xsl:attribute name="data-name">
            <xsl:value-of select="$fulldate" />
          </xsl:attribute>
          <div class="day_head">
            <xsl:value-of select="$ddate" />
          </div>
          <div class="day_content">
            <xsl:call-template name="build_day_content">
              <xsl:with-param name="date" select="$fulldate" />
            </xsl:call-template>
          </div>
        </xsl:if>
      </xsl:element>
    </xsl:if>

    <xsl:if test="(($ndx)+1) &lt;= ($day_right)">
      <xsl:call-template name="build_day">
        <xsl:with-param name="today" select="$today" />
        <xsl:with-param name="month" select="$month" />
        <xsl:with-param name="lastday" select="$lastday" />
        <xsl:with-param name="ndx" select="($ndx)+1" />
        <xsl:with-param name="date" select="($date)+1" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>


  <xsl:template name="build_weeks">
    <xsl:param name="today" />
    <xsl:param name="month" />
    <xsl:param name="lastday" />
    <xsl:param name="ndx" select="1" />
    <xsl:param name="date" />
    <tr>
      <xsl:call-template name="build_day">
        <xsl:with-param name="today" select="$today" />
        <xsl:with-param name="month" select="$month" />
        <xsl:with-param name="lastday" select="$lastday" />
        <xsl:with-param name="ndx" select="$ndx" />
        <xsl:with-param name="date" select="$date" />
      </xsl:call-template>
    </tr>

    <xsl:if test="($date)+7 &lt;= $lastday">
      <xsl:call-template name="build_weeks">
        <xsl:with-param name="today" select="$today" />
        <xsl:with-param name="month" select="$month" />
        <xsl:with-param name="lastday" select="$lastday" />
        <xsl:with-param name="date" select="($date)+7" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="build_calendar_head">
    <xsl:variable name="ndx_month" select="number(substring(@month,6,2))" />
    <xsl:variable name="ndx_year" select="number(substring(@month,1,4))" />
    <xsl:variable name="str_month" select="substring(string(100+$ndx_month),2,2)" />

    <xsl:variable name="month_next">
      <xsl:variable name="mon" select="($ndx_month mod 12)+1" />
      <xsl:variable name="year">
        <xsl:choose>
          <xsl:when test="$mon &gt; $ndx_month">
          <xsl:value-of select="$ndx_year" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="1 + $ndx_year" /></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:value-of select="concat($year,'-',substring(string(100+$mon),2,2),'-01')" />
    </xsl:variable>

    <xsl:variable name="month_last">
      <xsl:variable name="mon" select="(($ndx_month+10) mod 12)+1" />
      <xsl:variable name="year">
        <xsl:choose>
          <xsl:when test="$mon &gt; $ndx_month">
          <xsl:value-of select="-1+$ndx_year" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$ndx_year" /></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:value-of select="concat($year,'-',substring(string(100+$mon),2,2),'-01')" />
    </xsl:variable>

    <xsl:variable name="year_next">
      <xsl:value-of select="concat(1+$ndx_year,'-',$str_month,'-01')" />
    </xsl:variable>

    <xsl:variable name="year_last">
      <xsl:value-of select="concat(-1+$ndx_year,'-',$str_month,'-01')" />
    </xsl:variable>

    <tr>
      <td colspan="2" class="calnav cn_left">
        <xsl:element name="button">
          <xsl:attribute name="data-type">calmove</xsl:attribute>
          <xsl:attribute name="data-jump">
            <xsl:value-of select="$year_last" />
          </xsl:attribute>
          <xsl:attribute name="title">
            <xsl:value-of select="concat('last year (', $year_last, ')')" />
          </xsl:attribute>
          <xsl:text> &lt;&lt;&lt; </xsl:text>
        </xsl:element>
        <xsl:element name="button">
          <xsl:attribute name="data-type">calmove</xsl:attribute>
          <xsl:attribute name="data-jump">
            <xsl:value-of select="$month_last" />
          </xsl:attribute>
          <xsl:attribute name="title">
            <xsl:value-of select="concat('last month (', $month_last, ')')" />
          </xsl:attribute>
          <xsl:text> &lt; </xsl:text>
        </xsl:element>
      </td>
      <td colspan="3" class="cal_title">
        <xsl:call-template name="get_month_name">
          <xsl:with-param name="ndx" select="$ndx_month" />
        </xsl:call-template>
        <xsl:value-of select="$ndx_year" />
      </td>
      <td colspan="2" class="calnav cn_right">
        <xsl:element name="button">
          <xsl:attribute name="data-type">calmove</xsl:attribute>
          <xsl:attribute name="data-jump">
            <xsl:value-of select="$month_next" />
          </xsl:attribute>
          <xsl:attribute name="title">
            <xsl:value-of select="concat('next month (', $month_next, ')')" />
          </xsl:attribute>
          <xsl:text> &gt; </xsl:text>
        </xsl:element>
        <xsl:element name="button">
          <xsl:attribute name="data-type">calmove</xsl:attribute>
          <xsl:attribute name="data-jump">
            <xsl:value-of select="$year_next" />
          </xsl:attribute>
          <xsl:attribute name="title">
            <xsl:value-of select="concat('next year (', $year_next, ')')" />
          </xsl:attribute>
          <xsl:text> &gt;&gt;&gt; </xsl:text>
        </xsl:element>
      </td>
    </tr>
    <tr>
       <xsl:call-template name="make_day_heads" />
    </tr>
  </xsl:template>

  <xsl:template match="*[@month][@initialDay][@countOfDays]" mode="build_calendar">
    <xsl:param name="table_class" />
    <xsl:param name="sfw_class" select="'calendar'"  />

    <xsl:variable name="buttons" select="../buttons | ../schema/buttons" />

    <xsl:variable name="class">
      <xsl:text>Schema calendar</xsl:text>
      <xsl:if test="$table_class">
        <xsl:value-of select="concat(' ',$table_class)" />
      </xsl:if>
    </xsl:variable>

    <xsl:element name="table">
      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:apply-templates select="." mode="add_result_attribute" />
      <xsl:apply-templates select=".." mode="add_sfw_class_attribute" />
      <xsl:apply-templates select="." mode="add_on_cell_click_attribute" />
      <xsl:apply-templates select="../@calmove_base" mode="add_url_attribute" />
      <xsl:apply-templates select="." mode="add_url_attributes" />
      <thead>
        <xsl:apply-templates select="$buttons" mode="show_buttons" />
        <xsl:apply-templates select="." mode="build_calendar_head" />
      </thead>
      <tbody>
        <xsl:call-template name="build_weeks">
          <xsl:with-param name="today" select="@today" />
          <xsl:with-param name="month" select="@month" />
          <xsl:with-param name="lastday" select="@countOfDays" />
          <xsl:with-param name="date" select="1-(@initialDay)" />
        </xsl:call-template>
      </tbody>
    </xsl:element>
  </xsl:template>

  <xsl:template match="*[@month][@initialDay]" mode="make_dmdy">
    <xsl:param name="date" />

    <xsl:variable name="initialDay" select="@initialDay" />

    <xsl:variable name="ref" select="@month" />
    <xsl:variable name="yref" select="substring($ref,1,4)" />
    <xsl:variable name="mref" select="substring($ref,6,2)" />

    <xsl:variable name="ynum" select="substring($date,1,4)" />
    <xsl:variable name="mnum" select="substring($date,6,2)" />
    <xsl:variable name="dnum" select="substring($date,9,2)" />

    <xsl:variable name="dow" select="(-1 + $dnum + $initialDay) mod 7" />

    <xsl:call-template name="get_day_name">
      <xsl:with-param name="ndx" select="$dow" />
    </xsl:call-template>
    <xsl:text>, </xsl:text>
    <xsl:call-template name="get_month_name">
      <xsl:with-param name="ndx" select="$mnum" />
    </xsl:call-template>
    <xsl:value-of select="concat(' ',$dnum,' ',$ynum)" />
  </xsl:template>

  <xsl:template match="calendar[@rndx]">
    <xsl:apply-templates select="*[local-name()=current()/@row-name]" mode="build_calendar" />
  </xsl:template>

  <!--
      This template (and comment) will be discarded when sfw_calendar.xsl is
      imported to a stylesheet that already includes <xsl:template match="/">.
  -->
  <xsl:template match="/">
    <html>
      <head><title>Test Calendar</title></head>
      <body>
        <xsl:apply-templates select="*/*[@rndx='1']" mode="build_calendar" />
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>
