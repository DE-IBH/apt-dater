<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml">

  <xsl:output method="text"/>

  <xsl:template match="/apt-dater">
    <xsl:text>apt-dater host overview
=======================
</xsl:text>

    <xsl:for-each select="group">
      <xsl:call-template name="group"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="group">
    <xsl:text>
[</xsl:text><xsl:value-of select="@name"/><xsl:text>]
</xsl:text>

    <xsl:for-each select="host">
	<xsl:call-template name="host"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="host">
    <xsl:text> Hostname          : </xsl:text><xsl:value-of select="@hostname"/><xsl:text>
</xsl:text>
    <xsl:text> Status            : </xsl:text><xsl:value-of select="status"/><xsl:text>
</xsl:text>
    <xsl:if test="status != 'Unknown'">
    <xsl:text> LSB               : </xsl:text><xsl:value-of select="lsb/distri"/><xsl:text> </xsl:text><xsl:value-of select="lsb/release"/><xsl:text> </xsl:text><xsl:value-of select="lsb/codename"/><xsl:text>
</xsl:text>
    <xsl:text> Kernel            : </xsl:text><xsl:value-of select="kernel"/>
    <xsl:if test="kernel/@reboot = '1'">
      <xsl:text> (reboot needed)</xsl:text>
    </xsl:if>
    <xsl:text>
</xsl:text>
    <xsl:text> Packages installed: </xsl:text><xsl:value-of select="count(packages/pkg)"/><xsl:text>
</xsl:text>
    <xsl:text> Updates           : </xsl:text><xsl:value-of select="count(packages/pkg[@update = '1'])"/><xsl:text>
</xsl:text>
    <xsl:text> Packages hold back: </xsl:text><xsl:value-of select="count(packages/pkg[@onhold = '1'])"/><xsl:text>
</xsl:text>
    <xsl:text> Extra packages    : </xsl:text><xsl:value-of select="count(packages/pkg[@extra = '1'])"/><xsl:text>
</xsl:text>
    </xsl:if>

    <xsl:text>
</xsl:text>
  </xsl:template>

</xsl:stylesheet>
