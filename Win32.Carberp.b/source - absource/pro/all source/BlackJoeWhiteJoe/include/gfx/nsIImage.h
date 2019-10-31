/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsIImage_h___
#define nsIImage_h___

#include "nsISupports.h"
#include "nsMargin.h"
#include "nsRect.h"

class gfxASurface;
class gfxPattern;
class gfxMatrix;
class gfxRect;
class gfxContext;

class nsIDeviceContext;

struct nsColorMap
{
  //I lifted this from the image lib. The difference is that
  //this uses nscolor instead of NI_RGB. Multiple color pollution
  //is a bad thing. MMP
  PRInt32 NumColors;  // Number of colors in the colormap.
                      // A negative value can be used to denote a
                      // possibly non-unique set.
  //nscolor *Map;       // Colormap colors.
  PRUint8 *Index;     // NULL, if map is in index order, otherwise
                      // specifies the indices of the map entries. */
};

typedef enum {
    nsMaskRequirements_kNoMask,
    nsMaskRequirements_kNeeds1Bit,
    nsMaskRequirements_kNeeds8Bit
} nsMaskRequirements;


#define  nsImageUpdateFlags_kColorMapChanged 0x1
#define  nsImageUpdateFlags_kBitsChanged     0x2

// IID for the nsIImage interface
// c942f66c-97d0-470e-99de-a1efb4586afd
#define NS_IIMAGE_IID \
  { 0xc942f66c, 0x97d0, 0x470e, \
    { 0x99, 0xde, 0xa1, 0xef, 0xb4, 0x58, 0x6a, 0xfd } }

// Interface to Images
class nsIImage : public nsISupports
{

public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IIMAGE_IID)

  /**
   * Build and initialize the nsIImage
   * @param aWidth The width in pixels of the desired pixelmap
   * @param aHeight The height in pixels of the desired pixelmap
   * @param aDepth The number of bits per pixel for the pixelmap
   * @param aMaskRequirements A flag indicating if a alpha mask should be allocated 
   */
  virtual nsresult Init(PRInt32 aWidth, PRInt32 aHeight, PRInt32 aDepth, nsMaskRequirements aMaskRequirements) = 0;

  /**
   * Get the number of bytes per pixel for this image
   * @update - dwc 2/3/99
   * @return - the number of bytes per pixel
   */
  virtual PRInt32 GetBytesPix() = 0;

  /**
   * Get whether rows are organized top to bottom, or bottom to top 
   * @update - syd 3/29/99 
   * @return PR_TRUE if top to bottom, else PR_FALSE 
   */
  virtual PRBool GetIsRowOrderTopToBottom() = 0;

  /**
   * Get the width for the pixelmap
   * @update - dwc 2/1/99
   * @return The width in pixels for the pixelmap
   */
  virtual PRInt32 GetWidth() = 0;

  /**
   * Get the height for the pixelmap
   * @update - dwc 2/1/99
   * @return The height in pixels for the pixelmap
   */
  virtual PRInt32 GetHeight() = 0;

  /**
   * Get a pointer to the bits for the pixelmap, only if it is not optimized
   * @update - dwc 2/1/99
   * @return address of the DIB pixel array
   */
  virtual PRUint8 * GetBits() = 0;

  /**
   * Get the number of bytes needed to get to the next scanline for the pixelmap
   * @update - dwc 2/1/99
   * @return The number of bytes in each scanline
   */
  virtual PRInt32 GetLineStride() = 0;

  /**
   * Get whether this image has an alpha mask. Preferable to testing
   * if GetAlphaBits() is non-null.
   * @update - sfraser 10/19/99
   * @return PR_TRUE if the image has an alpha mask, PR_FALSE otherwise
   */
  virtual PRBool GetHasAlphaMask() = 0;

  /**
   * Get a pointer to the bits for the alpha mask
   * @update - dwc 2/1/99
   * @return address of the alpha mask pixel array
   */
  virtual PRUint8 * GetAlphaBits() = 0;

  /**
   * Get the number of bytes per scanline for the alpha mask
   * @update - dwc 2/1/99
   * @return The number of bytes in each scanline
   */
  virtual PRInt32 GetAlphaLineStride() = 0;

  /**
   * Update the nsIImage color table
   * @update - dougt 9/9/08
   * @param aFlags Used to pass in parameters for the update
   * @param aUpdateRect The rectangle to update
   * @return success code. failure means stop decoding
   */
  virtual nsresult ImageUpdated(nsIDeviceContext *aContext, PRUint8 aFlags, nsIntRect *aUpdateRect) = 0;
  
  /**
   * Get whether this image's region is completely filled with data.
   * @return PR_TRUE if image is complete, PR_FALSE if image is not yet 
   *         complete or broken
   */
  virtual PRBool GetIsImageComplete() = 0;

  /**
   * Converted this pixelmap to an optimized pixelmap for the device
   * @update - dwc 2/1/99
   * @param aContext The device to optimize for
   * @return the result of the operation, if NS_OK, then the pixelmap is optimized
   */
  virtual nsresult Optimize(nsIDeviceContext* aContext) = 0;

  /**
   * Get the colormap for the nsIImage
   * @update - dwc 2/1/99
   * @return if non null, the colormap for the pixelmap,otherwise the image is not color mapped
   */
  virtual nsColorMap * GetColorMap() = 0;

  /**
   * BitBlit the nsIImage to a device, the source and dest can be scaled.
   * @param aContext the destination
   * @param aUserSpaceToImageSpace the transform that maps user-space
   * coordinates to coordinates in (tiled, post-padding) image pixels
   * @param aFill the area to fill with tiled images
   * @param aPadding the padding to be added to this image before tiling,
   * in image pixels
   * @param aSubimage the subimage in padded+tiled image space that we're
   * extracting the contents from. Pixels outside this rectangle must not
   * be sampled.
   * 
   * So this is supposed to
   * -- add aPadding transparent pixels around the image
   * -- use that image to tile the plane
   * -- replace everything outside the aSubimage region with the nearest
   * border pixel of that region (like EXTEND_PAD)
   * -- fill aFill with the image, using aImageSpaceToDeviceSpace as the
   * image-space-to-device-space transform
   */
  virtual void Draw(gfxContext*        aContext,
                    const gfxMatrix&   aUserSpaceToImageSpace,
                    const gfxRect&     aFill,
                    const nsIntMargin& aPadding,
                    const nsIntRect&   aSubimage) = 0;

  /** 
   * Get the alpha depth for the image mask
   * @update - lordpixel 2001/05/16
   * @return  the alpha mask depth for the image, ie, 0, 1 or 8
   */
  virtual PRInt8 GetAlphaDepth() = 0;

  /**
   * Return information about the bits for this structure
   * @update - dwc 2/1/99
   * @return a bitmap info structure for the Device Dependent Bits
   */
  virtual void* GetBitInfo() = 0;


  /**
   * LockImagePixels
   * Lock the image pixels so that we can access them directly,
   * with safety. May be a noop on some platforms.
   *
   * If you want to be able to call GetSurface(), wrap the call in
   * LockImagePixels()/UnlockImagePixels(). This also allows you to write to
   * the surface returned by GetSurface().
   *
   * aMaskPixels = PR_TRUE for the mask, PR_FALSE for the image
   *
   * Must be balanced by a call to UnlockImagePixels().
   *
   * @update - sfraser 10/18/99
   * @return error result
   */
  NS_IMETHOD LockImagePixels(PRBool aMaskPixels) = 0;
  
  /**
   * UnlockImagePixels
   * Unlock the image pixels. May be a noop on some platforms.
   *
   * Should balance an earlier call to LockImagePixels().
   *
   * aMaskPixels = PR_TRUE for the mask, PR_FALSE for the image
   *
   * @update - sfraser 10/18/99
   * @return error result
   */
  NS_IMETHOD UnlockImagePixels(PRBool aMaskPixels) = 0;

  /**
   * GetSurface
   * Return the Thebes gfxASurface in aSurface, if there is one. Should be
   * wrapped by LockImagePixels()/UnlockImagePixels().
   *
   * aSurface will be AddRef'd (as with most getters), so
   * getter_AddRefs should be used.
   */
  NS_IMETHOD GetSurface(gfxASurface **aSurface) = 0;

  /**
   * GetSurface
   * Return the Thebes gfxPattern in aPattern. It is always possible to get a
   * gfxPattern (unlike the gfxASurface from GetSurface()).
   *
   * aPattern will be AddRef'd (as with most getters), so
   * getter_AddRefs should be used.
   */
  NS_IMETHOD GetPattern(gfxPattern **aPattern) = 0;

  /**
   * SetHasNoAlpha
   *
   * Hint to the image that all the pixels are fully opaque, even if
   * the original format requested a 1-bit or 8-bit alpha mask
   */
  virtual void SetHasNoAlpha() = 0;

  /**
   * Extract a rectangular region of the nsIImage and return it as a new
   * nsIImage.
   * @param aSubimage  the region to extract
   * @param aResult    the extracted image
   */
  NS_IMETHOD Extract(const nsIntRect& aSubimage,
                     nsIImage** aResult NS_OUTPARAM) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIImage, NS_IIMAGE_IID)

#endif
