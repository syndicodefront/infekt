/**
 * Copyright (C) 2023 syndicode
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#include "stdafx.h"
#include <json.hpp>
#include "nfo_renderer_export.h"

using json = nlohmann::json;

namespace nlohmann {
	template <>
	struct adl_serializer<std::wstring> {
		static void to_json(json& j, const std::wstring& str) {
			j = CUtil::FromWideStr(str, CP_UTF8);
		}

		static void from_json(const json& j, std::wstring& str) {
			str = CUtil::ToWideStr(j.get<std::string>(), CP_UTF8);
		}
	};
}

CNFOToHTMLCanvas::CNFOToHTMLCanvas()
	: CNFORenderer(false)
{
}

const std::string CNFOToHTMLCanvas::GetSettingsJSONString() const
{
	json j;

	j["blockWidth"] = GetSettings().uBlockWidth;
	j["blockHeight"] = GetSettings().uBlockHeight;
	j["colorBack"] = GetSettings().cBackColor.AsHex(false);
	j["colorText"] = GetSettings().cTextColor.AsHex(false);
	j["colorArt"] = GetSettings().cArtColor.AsHex(false);
	j["fontBold"] = GetSettings().bFontBold;
	j["shadowEnable"] = GetSettings().bGaussShadow;
	j["shadowRadius"] = GetSettings().uGaussBlurRadius;
	j["shadowColor"] = GetSettings().cGaussColor.AsHex(false);
	j["hyperlinksHighlight"] = GetSettings().bHilightHyperlinks;
	j["hyperlinksColor"] = GetSettings().cHyperlinkColor.AsHex(false);
	j["hyperlinksUnderline"] = GetSettings().bUnderlineHyperlinks;

	return j.dump();
}

const std::string CNFOToHTMLCanvas::GetRenderJSONString()
{
	if (!m_gridData && !CalculateGrid())
	{
		return {};
	}

	json j;

	j["width"] = m_nfo->GetGridWidth();
	j["height"] = m_nfo->GetGridHeight();
	j["blocks"] = json::array();
	j["links"] = json::array();
	j["text"] = json::array();

	std::string l_line_buf;
	l_line_buf.reserve(m_nfo->GetGridWidth());

	enum class ExportBlockIndices : unsigned int {
		FULL_BLOCK = 0,
		FULL_BLOCK_LIGHT_SHADE = 1,
		FULL_BLOCK_MEDIUM_SHADE = 2,
		FULL_BLOCK_DARK_SHADE = 3,
		LOWER_HALF = 4,
		UPPER_HALF = 5,
		RIGHT_HALF = 6,
		LEFT_HALF = 7,
		BLACK_SQUARE = 8,
		BLACK_SQUARE_SMALL = 9,
	};

	const CNFOHyperLink* l_buf_link = nullptr;

	for (size_t row = 0; row < m_nfo->GetGridHeight(); row++)
	{
		json j_row_blocks;

		j_row_blocks["row"] = row;
		j_row_blocks["b"] = json::array();

		size_t l_buf_first_col = (size_t)-1;
		size_t l_block_grp_col = 0;
		json j_block_grp;

		for (size_t col = 0; col < m_nfo->GetGridWidth(); col++)
		{
			const CRenderGridBlock& l_block = (*m_gridData)[row][col];

			if (l_block.shape != RGS_NO_BLOCK && l_block.shape != RGS_WHITESPACE_IN_TEXT)
			{
				if (l_block.shape != RGS_WHITESPACE)
				{
					// it's a block!

					if (!j_block_grp.empty() && col != l_block_grp_col + 1)
					{
						j_row_blocks["b"].push_back(j_block_grp);
						j_block_grp.clear();
					}

					if (j_block_grp.empty())
					{
						j_block_grp.push_back(col);
					}

					switch (l_block.shape)
					{
					case RGS_BLOCK_LOWER_HALF: j_block_grp.push_back(ExportBlockIndices::LOWER_HALF); break;
					case RGS_BLOCK_UPPER_HALF: j_block_grp.push_back(ExportBlockIndices::UPPER_HALF); break;
					case RGS_BLOCK_RIGHT_HALF: j_block_grp.push_back(ExportBlockIndices::RIGHT_HALF); break;
					case RGS_BLOCK_LEFT_HALF: j_block_grp.push_back(ExportBlockIndices::LEFT_HALF); break;
					case RGS_BLACK_SQUARE: j_block_grp.push_back(ExportBlockIndices::BLACK_SQUARE); break;
					case RGS_BLACK_SMALL_SQUARE: j_block_grp.push_back(ExportBlockIndices::BLACK_SQUARE_SMALL); break;
					case RGS_FULL_BLOCK:
						if (l_block.alpha < 100) j_block_grp.push_back(ExportBlockIndices::FULL_BLOCK_LIGHT_SHADE);
						else if (l_block.alpha < 150) j_block_grp.push_back(ExportBlockIndices::FULL_BLOCK_MEDIUM_SHADE);
						else if (l_block.alpha < 200) j_block_grp.push_back(ExportBlockIndices::FULL_BLOCK_DARK_SHADE);
						else j_block_grp.push_back(ExportBlockIndices::FULL_BLOCK);
						break;
					}

					l_block_grp_col = col;
				}

				if (l_buf_first_col != (size_t)-1)
				{
					l_line_buf += ' '; // add whitespace between non-whitespace chars that are skipped
				}
			}
			else
			{
				if (l_buf_first_col == (size_t)-1)
				{
					l_buf_first_col = col;
				}

				const auto l_link = m_nfo->GetLink(row, col);
				const bool now_in_link = (l_link != nullptr);
				const bool l_buf_is_link = (l_buf_link != nullptr);

				if (now_in_link != l_buf_is_link)
				{
					if (!l_line_buf.empty())
					{
						CUtil::StrTrimRight(l_line_buf);

						if (l_buf_is_link)
						{
							j["links"].emplace_back(json{ {"row", row}, {"col", l_buf_first_col}, {"t", l_line_buf}, {"href", l_buf_link->GetHref()} });
						}
						else
						{
							j["text"].emplace_back(json{ {"row", row}, {"col", l_buf_first_col}, {"t", l_line_buf} });
						}

						l_line_buf.clear();
						l_buf_link = nullptr;
					}

					l_buf_first_col = col;
					l_buf_link = l_link;
				}

				l_line_buf += m_nfo->GetGridCharUtf8(row, col);
			}
		}

		if (!j_block_grp.empty())
		{
			j_row_blocks["b"].push_back(j_block_grp);
		}

		if (!j_row_blocks["b"].empty())
		{
			j["blocks"].push_back(j_row_blocks);
		}

		if (l_buf_first_col != (size_t)-1)
		{
			CUtil::StrTrimRight(l_line_buf);

			if (l_buf_link != nullptr)
			{
				j["links"].emplace_back(json{ {"row", row}, {"col", l_buf_first_col}, {"t", l_line_buf}, {"href", CUtil::FromWideStr(l_buf_link->GetHref(), CP_UTF8)} });
			}
			else
			{
				j["text"].emplace_back(json{ {"row", row}, {"col", l_buf_first_col}, {"t", l_line_buf} });
			}
		}

		l_line_buf.clear();
		l_buf_link = nullptr;
	}

	return j.dump();
}

const std::string CNFOToHTMLCanvas::GetRenderCodeString() const
{
	return R"(class {
    #container;
    #nfoData;
    #renderSettings;
    #strides = [];

    constructor(container, nfoData, renderSettings) {
        this.#container = container;
        this.#nfoData = nfoData;
        this.#renderSettings = renderSettings;
        this.#setUpContainer();
        this.#createText();
    }

    get padding() {
        const MIN = 8;
        const settings = this.#renderSettings;
        return settings.shadowEnable && settings.shadowRadius > MIN ? settings.shadowRadius : MIN;
    }

    #setUpContainer() {
        const containerId = 'nfo-' + Math.round(Date.now() * Math.random()).toString(36);
        const style = document.head.appendChild(document.createElement('style'));

        style.sheet.insertRule('#' + containerId + '{ position: relative; overflow: hidden; padding: 0; ' +
            'background-color: #' + this.#renderSettings.colorBack + '; color: #' + this.#renderSettings.colorText + '; }');
        style.sheet.insertRule('#' + containerId + ' > canvas { position: absolute; margin: 0; padding: 0; /*outline: 1px solid green;*/ }');
        style.sheet.insertRule('#' + containerId + ' > span, #' + containerId + ' > a {' +
            'position: absolute;' +
            'margin: 0; padding: 0;' +
            'white-space: pre;' +
            'font-family: "SF Mono";' +
            'font-size: 13px;' +
            'font-weight: ' + (this.#renderSettings.fontBold ? 'bold' : 'normal') + ';' +
            'text-rendering: geometricPrecision;' +
            'font-kerning: none; font-strech: normal; font-size-adjust: none; font-optical-sizing: none;' +
            'text-size-adjust: 100%; -webkit-text-size-adjust: 100%;' +
            'user-select: none;' +
            '}');

        const hyperlinkColor = '#' + (this.#renderSettings.hyperlinksHighlight ? this.#renderSettings.hyperlinksColor : this.#renderSettings.colorText);
        const textDecoration = this.#renderSettings.hyperlinksHighlight && this.#renderSettings.hyperlinksUnderline ? 'underline ' + hyperlinkColor : 'none';

        style.sheet.insertRule('#' + containerId + ' > a { color: ' + hyperlinkColor + '; text-decoration: ' + textDecoration + '; }');

        this.#container.id = containerId;
        this.#container.style.width = (this.#nfoData.width * this.#renderSettings.blockWidth + this.padding * 2) + 'px';
        this.#container.style.height = (this.#nfoData.height * this.#renderSettings.blockHeight + this.padding * 2) + 'px';

        this.#setUpStrides();
    }

    #setUpStrides() {
        if (this.#nfoData.blocks.length < 1) {
            return;
        }

        const MAX_PIXELS = 4096 * 4096;
        const MAX_DIMENSION = 32767;
        const MIN_LINES = 5;
        const MIN_STRIDES = 5;

        const totalHeightPixels = NfoDevicePixelRatio() * parseInt(this.#container.style.height);
        const totalWidthPixels = NfoDevicePixelRatio() * parseInt(this.#container.style.width);
        const strideExtraPixels = this.#renderSettings.blockHeight + this.padding;

        if (totalWidthPixels > MAX_DIMENSION) {
            throw new Error("NFO width exceeds browser limits");
        }

        let strideCount = MIN_STRIDES;
        let supposedStrideHeightPixels;

        do {
            ++strideCount;

            supposedStrideHeightPixels = Math.ceil((this.#renderSettings.blockHeight * this.#nfoData.height) / strideCount) + strideExtraPixels;

            if (strideCount > 1 && supposedStrideHeightPixels < this.#renderSettings.blockHeight * MIN_LINES) {
                --strideCount;
                break;
            }
        } while (supposedStrideHeightPixels * totalWidthPixels > MAX_PIXELS || supposedStrideHeightPixels > MAX_DIMENSION);

        const strideHeightInLines = Math.ceil(this.#nfoData.height / strideCount);

        for (let stride = 0; stride < strideCount; ++stride) {
            const canvas = document.createElement('canvas');

            canvas.style.left = 0;
            canvas.style.width = this.#container.style.width;

            canvas.style.top = (stride === 0 ? 0 : this.padding + stride * strideHeightInLines * this.#renderSettings.blockHeight) + 'px';
            canvas.style.height = (strideHeightInLines * this.#renderSettings.blockHeight)
                + (stride === 0 || stride === strideCount - 1 ? this.padding : 0) + 'px';

            this.#container.appendChild(canvas);

            const firstLine = stride * strideHeightInLines - 1;
            const lastLine = (stride + 1) * strideHeightInLines + 1;
            const strideBlocks = this.#nfoData.blocks.filter(
                b => (b.row >= firstLine && b.row <= lastLine)
            );

            this.#strides.push(new NfoRendererStride(canvas, strideBlocks, firstLine));
        }
    }

    render() {
        for (let i = 0; i < this.#strides.length; ++i) {
            const paddingY = (i === 0 ? this.padding : 0);

            this.#strides[i].render(
                this.#renderSettings,
                this.padding,
                paddingY
            );
        }
    }

    #createText() {
        const textToDom = (input, dom) => {
            const x = this.padding + input.col * this.#renderSettings.blockWidth;
            const y = this.padding + input.row * this.#renderSettings.blockHeight;

            dom.style.left = x + 'px';
            dom.style.top = y + 'px';

            dom.appendChild(document.createTextNode(input.t));
            this.#container.appendChild(dom);
        }

        for (const textLine of this.#nfoData.text) {
            textToDom(textLine, document.createElement('span'));
        }

        for (const link of this.#nfoData.links) {
            const a = document.createElement('a');

            a.href = link.href;
            a.target = '_blank';

            textToDom(link, a);
        }
    }
};

const NfoDevicePixelRatio = function () {
    const MIN = 1;
    const MAX = 4;

    let devicePixelRatio = window.devicePixelRatio || 1;

    devicePixelRatio = devicePixelRatio < MIN ? MIN : Math.round(devicePixelRatio);
    devicePixelRatio = devicePixelRatio > MAX ? MAX : devicePixelRatio;

    return devicePixelRatio;
};

const NfoRendererStride = class {
    #targetCanvas;
    #virtualCanvas;
    #blocks;
    #offsetLines;
    #oldDevicePixelRatio = 0;
    #ctx = null;

    constructor(targetCanvas, blocks, offsetLines) {
        this.#targetCanvas = targetCanvas;
        this.#virtualCanvas = null;
        this.#blocks = blocks;
        this.#offsetLines = offsetLines;
    }

    render(renderSettings, paddingX, paddingY) {
        if (NfoDevicePixelRatio() !== this.#oldDevicePixelRatio) {
            this.#setUpCanvas();
        }

        this.#renderBackground();

        if (renderSettings.shadowEnable && renderSettings.shadowRadius >= 1) {
            const shadowRadius = Math.min(renderSettings.shadowRadius, renderSettings.blockHeight);

            if (this.#ctx.filter !== undefined) {
                this.#ctx.filter = 'blur(' + Math.round(shadowRadius * 0.5) + 'px)';
                this.#renderBlocks(renderSettings.shadowColor, paddingX, paddingY);
                this.#ctx.filter = 'none';
                this.#renderBlocks(renderSettings.colorBack, paddingX, paddingY);
            } else {
                this.#ctx.shadowColor = '#' + renderSettings.shadowColor;
                this.#ctx.shadowBlur = shadowRadius;
                this.#ctx.shadowOffsetX = 0;
                this.#ctx.shadowOffsetY = 0;
            }
        }

        this.#renderBlocks(renderSettings.colorArt, paddingX, paddingY);
        this.#ctx.shadowBlur = 0;

        const targetCtx = this.#targetCanvas.getContext('2d', { alpha: false });
        const clipWidth = this.#virtualCanvas.width;
        const clipHeight = this.#virtualCanvas.height - 2 * renderSettings.blockHeight;

        targetCtx.drawImage(this.#virtualCanvas, 0, renderSettings.blockHeight, clipWidth, clipHeight, 0, 0, clipWidth, clipHeight);
    }

    #renderBackground() {
        this.#ctx.fillStyle = '#' + renderSettings.colorBack;
        this.#ctx.fillRect(0, 0, this.#virtualCanvas.width, this.#virtualCanvas.height);
    }

    #renderBlocks(colorHex, paddingX, paddingY) {
        const BLOCK_WIDTH = renderSettings.blockWidth;
        const BLOCK_HEIGHT = renderSettings.blockHeight;
        const HALF_BLOCK_WIDTH = renderSettings.blockWidth * 0.5;
        const HALF_BLOCK_HEIGHT = renderSettings.blockHeight * 0.5;

        const BT = {
            FULL_BLOCK: 0,
            FULL_BLOCK_LIGHT_SHADE: 1,
            FULL_BLOCK_MEDIUM_SHADE: 2,
            FULL_BLOCK_DARK_SHADE: 3,
            LOWER_HALF: 4,
            UPPER_HALF: 5,
            RIGHT_HALF: 6,
            LEFT_HALF: 7,
            BLACK_SQUARE: 8,
            BLACK_SQUARE_SMALL: 9,
        };

        const pathsByAlpha = new Map();

        for (const rowBlocks of this.#blocks) {
            const rowY = paddingY + (rowBlocks.row - this.#offsetLines) * BLOCK_HEIGHT;

            for (const blockSpan of rowBlocks.b) {
                const firstCol = blockSpan[0];

                for (let j = 1; j < blockSpan.length; ++j) {
                    let blockX = paddingX + (firstCol + j - 1) * BLOCK_WIDTH;
                    let blockY = rowY;
                    let blockWidth = BLOCK_WIDTH;
                    let blockHeight = BLOCK_HEIGHT;
                    let alpha = 255;

                    switch (blockSpan[j]) {
                        case BT.LOWER_HALF: blockY += HALF_BLOCK_HEIGHT; /* fallthrough */
                        case BT.UPPER_HALF: blockHeight = HALF_BLOCK_HEIGHT; break;
                        case BT.RIGHT_HALF: blockX += HALF_BLOCK_WIDTH; /* fallthrough */
                        case BT.LEFT_HALF: blockWidth = HALF_BLOCK_WIDTH; break;
                        case BT.BLACK_SQUARE:
                            blockWidth = blockHeight = BLOCK_WIDTH * 0.75;
                            blockY += HALF_BLOCK_HEIGHT - blockHeight * 0.5;
                            blockX += HALF_BLOCK_WIDTH - blockWidth * 0.5;
                            break;
                        case BT.BLACK_SQUARE_SMALL:
                            blockWidth = blockHeight = HALF_BLOCK_WIDTH;
                            blockY += HALF_BLOCK_HEIGHT - blockHeight * 0.5;
                            blockX += HALF_BLOCK_WIDTH - blockWidth * 0.5;
                            break;
                        case BT.FULL_BLOCK_LIGHT_SHADE: alpha = 90; break;
                        case BT.FULL_BLOCK_MEDIUM_SHADE: alpha = 140; break;
                        case BT.FULL_BLOCK_DARK_SHADE: alpha = 190; break;
                    }

                    if (!pathsByAlpha.has(alpha)) {
                        pathsByAlpha.set(alpha, new Path2D());
                    }

                    pathsByAlpha.get(alpha).rect(blockX, blockY, blockWidth, blockHeight);
                }
            }
        }

        for (const [alpha, path] of pathsByAlpha) {
            this.#ctx.fillStyle = '#' + colorHex.substring(0, 6) + alpha.toString(16);
            this.#ctx.fill(path);
        }
    }

    #setUpCanvas() {
        const devicePixelRatio = NfoDevicePixelRatio();
        const rect = this.#targetCanvas.getBoundingClientRect();

        this.#targetCanvas.width = rect.width * devicePixelRatio;
        this.#targetCanvas.height = rect.height * devicePixelRatio;

        const virtualCanvasWidth = this.#targetCanvas.width;
        const virtualCanvasHeight = this.#targetCanvas.height + 2 * renderSettings.blockHeight;
        const offscreenCanvas = new OffscreenCanvas(this.#targetCanvas.width, virtualCanvasHeight, virtualCanvasWidth);

        if ('filter' in offscreenCanvas.getContext('2d', { alpha: false })) {
            this.#virtualCanvas = offscreenCanvas;
        } else {
            this.#virtualCanvas = document.createElement('canvas');
            this.#virtualCanvas.width = virtualCanvasWidth;
            this.#virtualCanvas.height = virtualCanvasHeight;
        }

        this.#ctx = this.#virtualCanvas.getContext('2d', { alpha: false });
        this.#ctx.scale(devicePixelRatio, devicePixelRatio);
        this.#oldDevicePixelRatio = devicePixelRatio;
    }
})";
}

const std::string CNFOToHTMLCanvas::GetFullHTML()
{
	std::stringstream ss;

	ss << R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title></title>
    <link href="https://fonts.cdnfonts.com/css/sf-mono" rel="stylesheet">
</head>
<body>
<div id="nfo"></div>
<script>
)";

	ss << "const renderSettings = " << GetSettingsJSONString() << ";\n";
	ss << "const nfoData = " << GetRenderJSONString() << ";\n";
	ss << "const NfoRenderer = " << GetRenderCodeString() << ";\n";

	ss << R"(

document.addEventListener('DOMContentLoaded', function () {
    const renderer = new NfoRenderer(document.getElementById('nfo'), nfoData, renderSettings);

    if (window.matchMedia) {
        (function updatePixelRatio() {
            matchMedia('(resolution: ' + window.devicePixelRatio + 'dppx)')
                .addEventListener('change', updatePixelRatio, { once: true });

            renderer.render();
        })();
    } else {
        renderer.render();
    }
});
</script>
</body>
</html>)";

	return ss.str();
}
