<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="text"/>

<!-- root rule -->
<xsl:template match="/">
   <xsl:apply-templates/>
</xsl:template>

<!-- supress non-selected nodes-->
<!-- <xsl:template match="*"/> -->


<xsl:template match="table">
<xsl:apply-templates select="tr"/>
</xsl:template>


<xsl:template match="tr">
<xsl:if test="string(number(td[1])) != 'NaN'"> <!-- output only if first column contains valid number -->
	<xsl:for-each select="td[position() != 2]"> <!-- for each column except second (don't need it) -->
		<xsl:choose>
			<xsl:when test="contains(normalize-space(.), ' ')">
				<xsl:value-of select="concat('&quot;', normalize-space(.), '&quot;')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="normalize-space(.)"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="position() != last()">
			<xsl:value-of select="';'"/>
		</xsl:if>
	</xsl:for-each>
	<xsl:text>&#10;</xsl:text>
</xsl:if>
</xsl:template>

<!--
<xsl:template match="tr">
<xsl:apply-templates select="td"/>
<xsl:text>
</xsl:text>
</xsl:template>


<xsl:template match="td">;<xsl:value-of select="normalize-space(.)" /></xsl:template>
-->

</xsl:stylesheet>
