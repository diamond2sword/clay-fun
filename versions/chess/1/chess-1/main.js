const CLAY_RENDER_COMMAND_TYPE_NONE = 0;
const CLAY_RENDER_COMMAND_TYPE_RECTANGLE = 1;
const CLAY_RENDER_COMMAND_TYPE_BORDER = 2;
const CLAY_RENDER_COMMAND_TYPE_TEXT = 3;
const CLAY_RENDER_COMMAND_TYPE_IMAGE = 4;
const CLAY_RENDER_COMMAND_TYPE_SCISSOR_START = 5;
const CLAY_RENDER_COMMAND_TYPE_SCISSOR_END = 6;
const CLAY_RENDER_COMMAND_TYPE_CUSTOM = 7;
const GLOBAL_FONT_SCALING_FACTOR = 0.8;
let renderCommandSize = 0;
let scratchSpaceAddress = 8;
let heapSpaceAddress = 0;
let memoryDataView;
let textDecoder = new TextDecoder("utf-8");
let previousFrameTime;
let fontsById = [
	'JetBrainsMono',
];
let elementCache = {};
let imageCache = {};
let colorDefinition = { type: 'struct', members: [
	{name: 'r', type: 'float' },
	{name: 'g', type: 'float' },
	{name: 'b', type: 'float' },
	{name: 'a', type: 'float' },
]};
let stringDefinition = { type: 'struct', members: [
	{name: 'length', type: 'uint32_t' },
	{name: 'chars', type: 'uint32_t' },
]};
let borderDefinition = { type: 'struct', members: [
	{name: 'width', type: 'uint32_t'},
	{name: 'color', ...colorDefinition},
]};
let cornerRadiusDefinition = { type: 'struct', members: [
	{name: 'topLeft', type: 'float'},
	{name: 'topRight', type: 'float'},
	{name: 'bottomLeft', type: 'float'},
	{name: 'bottomRight', type: 'float'},
]};
let rectangleConfigDefinition = { name: 'rectangle', type: 'struct', members: [
	{ name: 'color', ...colorDefinition },
	{ name: 'cornerRadius', ...cornerRadiusDefinition },
	{ name: 'link', ...stringDefinition },
	{ name: 'cursorPointer', type: 'uint8_t' },
]};
let borderConfigDefinition = { name: 'text', type: 'struct', members: [
	{ name: 'left', ...borderDefinition },
	{ name: 'right', ...borderDefinition },
	{ name: 'top', ...borderDefinition },
	{ name: 'bottom', ...borderDefinition },
	{ name: 'betweenChildren', ...borderDefinition },
	{ name: 'cornerRadius', ...cornerRadiusDefinition }
]};
let textConfigDefinition = { name: 'text', type: 'struct', members: [
   { name: 'textColor', ...colorDefinition },
   { name: 'fontId', type: 'uint16_t' },
   { name: 'fontSize', type: 'uint16_t' },
   { name: 'letterSpacing', type: 'uint16_t' },
   { name: 'lineSpacing', type: 'uint16_t' },
   { name: 'wrapMode', type: 'uint32_t' },
   { name: 'disablePointerEvents', type: 'uint8_t' }
]};
let scrollConfigDefinition = { name: 'text', type: 'struct', members: [
	{ name: 'horizontal', type: 'bool' },
	{ name: 'vertical', type: 'bool' },
]};
let boundingBoxDefinition = { type: 'struct', members: [
	{ name: 'x', type: 'float' },
	{ name: 'y', type: 'float' },
	{ name: 'width', type: 'float' },
	{ name: 'height', type: 'float' },
]}
let imageConfigDefinition = { name: 'image', type: 'struct', members: [
	{ name: 'imageData', type: 'uint32_t' },
	{ name: 'sourceDimensions', type: 'struct', members: [
		{ name: 'width', type: 'float' },
		{ name: 'height', type: 'float' },
	]},
	{ name: 'sourceURL', ...stringDefinition },
	{ name: 'subrectangle', ...boundingBoxDefinition },
]};
let customConfigDefinition = { name: 'custom', type: 'struct', members: [
	{ name: 'customData', type: 'uint32_t' },
]}
let renderCommandDefinition = {
	name: 'CLay_RenderCommand',
	type: 'struct',
	members: [
		{ name: 'boundingBox', ...boundingBoxDefinition },
		{ name: 'config', type: 'uint32_t'},
		{ name: 'text', ...stringDefinition },
		{ name: 'id', type: 'uint32_t' },
		{ name: 'commandType', type: 'uint32_t', },
	]
};

function getStructTotalSize(definition) {
	switch(definition.type) {
		case 'union':
		case 'struct': {
			let totalSize = 0;
			for (const member of definition.members) {
				let result = getStructTotalSize(member);
				if (definition.type === 'struct') {
					totalSize += result;
				} else {
					totalSize = Math.max(totalSize, result);
				}
			}
			return totalSize;
		}
		case 'float': return 4;
		case 'uint32_t': return 4;
		case 'int32_t': return 4;
		case 'uint16_t': return 2;
		case 'uint8_t': return 1;
		case 'bool': return 1;
		default: {
			throw "Unimplemented C data type " + definition.type
		}
	}
}

function readStructAtAddress(address, definition) {
	switch(definition.type) {
		case 'union':
		case 'struct': {
			let struct = { __size: 0 };
			for (const member of definition.members) {
				let result = readStructAtAddress(address, member);
				struct[member.name] = result;
				if (definition.type === 'struct') {
					struct.__size += result.__size;
					address += result.__size;
				} else {
					struct.__size = Math.max(struct.__size, result.__size);
				}
			}
			return struct;
		}
		case 'float': return { value: memoryDataView.getFloat32(address, true), __size: 4 };
		case 'uint32_t': return { value: memoryDataView.getUint32(address, true), __size: 4 };
		case 'int32_t': return { value: memoryDataView.getUint32(address, true), __size: 4 };
		case 'uint16_t': return { value: memoryDataView.getUint16(address, true), __size: 2 };
		case 'uint8_t': return { value: memoryDataView.getUint8(address, true), __size: 1 };
		case 'bool': return { value: memoryDataView.getUint8(address, true), __size: 1 };
		default: {
			throw "Unimplemented C data type " + definition.type
		}
	}
}

function getTextDimensions(text, font) {
	// re-use canvas object for better performance
	window.canvasContext.font = font;
	let metrics = window.canvasContext.measureText(text);
	return { width: metrics.width, height: metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent };
}

function createMainArena(arenaStructAddress, arenaMemoryAddress) {
	let memorySize = instance.exports.Clay_MinMemorySize();
	// Last arg is address to store return value
	instance.exports.Clay_CreateArenaWithCapacityAndMemory(arenaStructAddress, memorySize, arenaMemoryAddress);
}
async function init() {
	await Promise.all(fontsById.map(f => document.fonts.load(`12px "${f}"`)));
	window.htmlRoot = document.body.appendChild(document.createElement('div'));
	window.canvasRoot = document.body.appendChild(document.createElement('canvas'));
	window.canvasContext = window.canvasRoot.getContext("2d");
	window.mousePositionXThisFrame = 0;
	window.mousePositionYThisFrame = 0;
	window.mouseWheelXThisFrame = 0;
	window.mouseWheelYThisFrame = 0;
	window.touchDown = false;
	window.arrowKeyDownPressedThisFrame = false;
	window.arrowKeyUpPressedThisFrame = false;
	let zeroTimeout = null;
	document.addEventListener("wheel", (event) => {
		window.mouseWheelXThisFrame = event.deltaX * -0.1;
		window.mouseWheelYThisFrame = event.deltaY * -0.1;
		clearTimeout(zeroTimeout);
		zeroTimeout = setTimeout(() => {
			window.mouseWheelXThisFrame = 0;
			window.mouseWheelYThisFrame = 0;
		}, 10);
	});

	function handleTouch (event) {
		if (event.touches.length === 1) {
			window.touchDown = true;
			let target = event.target;
			let scrollTop = 0;
			let scrollLeft = 0;
			/*
			let activeRendererIndex = memoryDataView.getUint32(instance.exports.ACTIVE_RENDERER_INDEX.value, true);
			while (activeRendererIndex !== 1 && target) {
				scrollLeft += target.scrollLeft;
				scrollTop += target.scrollTop;
				target = target.parentElement;
			}
			*/
			window.mousePositionXThisFrame = event.changedTouches[0].pageX + scrollLeft;
			window.mousePositionYThisFrame = event.changedTouches[0].pageY + scrollTop;
		}
	}

	document.addEventListener("touchstart", handleTouch);
	document.addEventListener("touchmove", handleTouch);
	document.addEventListener("touchend", () => {
		window.touchDown = false;
		window.mousePositionXThisFrame = 0;
		window.mousePositionYThisFrame = 0;
	})

	document.addEventListener("mousemove", (event) => {
		let target = event.target;
		let scrollTop = 0;
		let scrollLeft = 0;
		/*
		let activeRendererIndex = memoryDataView.getUint32(instance.exports.ACTIVE_RENDERER_INDEX.value, true);
		while (activeRendererIndex !== 1 && target) {
			scrollLeft += target.scrollLeft;
			scrollTop += target.scrollTop;
			target = target.parentElement;
		}
		*/
		window.mousePositionXThisFrame = event.x + scrollLeft;
		window.mousePositionYThisFrame = event.y + scrollTop;
	});

	document.addEventListener("mousedown", (event) => {
		window.mouseDown = true;
		window.mouseDownThisFrame = true;
	});

	document.addEventListener("mouseup", (event) => {
		window.mouseDown = false;
	});

	document.addEventListener("keydown", (event) => {
		if (event.key === "ArrowDown") {
			window.arrowKeyDownPressedThisFrame = true;
		}
		if (event.key === "ArrowUp") {
			window.arrowKeyUpPressedThisFrame = true;
		}
		if (event.key === "d") {
			window.dKeyPressedThisFrame = true;
		}
	});

	const importObject = {
		clay: {
			measureTextFunction: (addressOfDimensions, textToMeasure, addressOfConfig) => {
				let stringLength = memoryDataView.getUint32(textToMeasure, true);
				let pointerToString = memoryDataView.getUint32(textToMeasure + 4, true);
				let textConfig = readStructAtAddress(addressOfConfig, textConfigDefinition);
				let textDecoder = new TextDecoder("utf-8");
				let text = textDecoder.decode(memoryDataView.buffer.slice(pointerToString, pointerToString + stringLength));
				let sourceDimensions = getTextDimensions(text, `${Math.round(textConfig.fontSize.value * GLOBAL_FONT_SCALING_FACTOR)}px ${fontsById[textConfig.fontId.value]}`);
				memoryDataView.setFloat32(addressOfDimensions, sourceDimensions.width, true);
				memoryDataView.setFloat32(addressOfDimensions + 4, sourceDimensions.height, true);
			},
			queryScrollOffsetFunction: (addressOfOffset, elementId) => {
				let container = document.getElementById(elementId.toString());
				if (container) {
					memoryDataView.setFloat32(addressOfOffset, -container.scrollLeft, true);
					memoryDataView.setFloat32(addressOfOffset + 4, -container.scrollTop, true);
				}
			}
		},
	};
	const { instance } = await WebAssembly.instantiateStreaming(
		fetch("/clay/index.wasm"), importObject
	);
	memoryDataView = new DataView(new Uint8Array(instance.exports.memory.buffer).buffer);
	scratchSpaceAddress = instance.exports.__heap_base.value;
	heapSpaceAddress = instance.exports.__heap_base.value + 1024;
	let arenaAddress = scratchSpaceAddress + 8;
	window.instance = instance;
	createMainArena(arenaAddress, heapSpaceAddress);
	memoryDataView.setFloat32(instance.exports.__heap_base.value, window.innerWidth, true);
	memoryDataView.setFloat32(instance.exports.__heap_base.value + 4, window.innerHeight, true);
	instance.exports.Clay_Initialize(arenaAddress, instance.exports.__heap_base.value);
	renderCommandSize = getStructTotalSize(renderCommandDefinition);
	renderLoop();
}

function MemoryIsDifferent(one, two, length) {
	for (let i = 0; i < length; i++) {
		if (one[i] !== two[i]) {
			return true;
		}
	}
	return false;
}
/*
function renderLoopHTML() {
	let capacity = memoryDataView.getInt32(scratchSpaceAddress, true);
	let length = memoryDataView.getInt32(scratchSpaceAddress + 4, true);
	let arrayOffset = memoryDataView.getUint32(scratchSpaceAddress + 8, true);
	let scissorStack = [{ nextAllocation: { x: 0, y: 0 }, element: htmlRoot, nextElementIndex: 0 }];
	let previousId = 0;
	for (let i = 0; i < length; i++, arrayOffset += renderCommandSize) {
		let entireRenderCommandMemory = new Uint32Array(memoryDataView.buffer.slice(arrayOffset, arrayOffset + renderCommandSize));
		let renderCommand = readStructAtAddress(arrayOffset, renderCommandDefinition);
		let parentElement = scissorStack[scissorStack.length - 1];
		let element = null;
		let isMultiConfigElement = previousId === renderCommand.id.value;
		if (!elementCache[renderCommand.id.value]) {
			let elementType = 'div';
			switch (renderCommand.commandType.value) {
				case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
					if (readStructAtAddress(renderCommand.config.value, rectangleConfigDefinition).link.length.value > 0) {
						elementType = 'a';
					}
					break;
				}
				case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
					elementType = 'img'; break;
				}
				default: break;
			}
			element = document.createElement(elementType);
			element.id = renderCommand.id.value;
			if (renderCommand.commandType.value === CLAY_RENDER_COMMAND_TYPE_SCISSOR_START) {
				element.style.overflow = 'hidden';
			}
			elementCache[renderCommand.id.value] = {
				exists: true,
				element: element,
				previousMemoryCommand: new Uint8Array(0),
				previousMemoryConfig: new Uint8Array(0),
				previousMemoryText: new Uint8Array(0)
			};
		}

		let elementData = elementCache[renderCommand.id.value];
		element = elementData.element;
		if (!isMultiConfigElement && Array.prototype.indexOf.call(parentElement.element.children, element) !== parentElement.nextElementIndex) {
			if (parentElement.nextElementIndex === 0) {
				parentElement.element.insertAdjacentElement('afterbegin', element);
			} else {
				parentElement.element.childNodes[Math.min(parentElement.nextElementIndex - 1, parentElement.element.childNodes.length - 1)].insertAdjacentElement('afterend', element);
			}
		}

		elementData.exists = true;
		// Don't get me started. Cheaper to compare the render command memory than to update HTML elements
		let dirty = MemoryIsDifferent(elementData.previousMemoryCommand, entireRenderCommandMemory, renderCommandSize) && !isMultiConfigElement;
		if (!isMultiConfigElement) {
			parentElement.nextElementIndex++;
		}

		previousId = renderCommand.id.value;

		elementData.previousMemoryCommand = entireRenderCommandMemory;
		let offsetX = scissorStack.length > 0 ? scissorStack[scissorStack.length - 1].nextAllocation.x : 0;
		let offsetY = scissorStack.length > 0 ? scissorStack[scissorStack.length - 1].nextAllocation.y : 0;
		if (dirty) {
			element.style.transform = `translate(${Math.round(renderCommand.boundingBox.x.value - offsetX)}px, ${Math.round(renderCommand.boundingBox.y.value - offsetY)}px)`
			element.style.width = Math.round(renderCommand.boundingBox.width.value) + 'px';
			element.style.height = Math.round(renderCommand.boundingBox.height.value) + 'px';
		}

		// note: commandType is packed to uint8_t and has 3 garbage bytes of padding
		switch(renderCommand.commandType.value & 0xff) {
			case (CLAY_RENDER_COMMAND_TYPE_NONE): {
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_RECTANGLE): {
				let config = readStructAtAddress(renderCommand.config.value, rectangleConfigDefinition);
				let configMemory = new Uint8Array(memoryDataView.buffer.slice(renderCommand.config.value, renderCommand.config.value + config.__size));
				let linkContents = config.link.length.value > 0 ? textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(config.link.chars.value, config.link.chars.value + config.link.length.value))) : 0;
				memoryDataView.setUint32(0, renderCommand.id.value, true);
				if (linkContents.length > 0 && (window.mouseDownThisFrame || window.touchDown) && instance.exports.Clay_PointerOver(0)) {
					window.location.href = linkContents;
				}
				if (!dirty && !MemoryIsDifferent(configMemory, elementData.previousMemoryConfig, config.__size)) {
					break;
				}
				if (linkContents.length > 0) {
					element.href = linkContents;
				}

				if (linkContents.length > 0 || config.cursorPointer.value) {
					element.style.pointerEvents = 'all';
					element.style.cursor = 'pointer';
				}
				elementData.previousMemoryConfig = configMemory;
				let color = config.color;
				element.style.backgroundColor = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
				if (config.cornerRadius.topLeft.value > 0) {
					element.style.borderTopLeftRadius = config.cornerRadius.topLeft.value + 'px';
				}
				if (config.cornerRadius.topRight.value > 0) {
					element.style.borderTopRightRadius = config.cornerRadius.topRight.value + 'px';
				}
				if (config.cornerRadius.bottomLeft.value > 0) {
					element.style.borderBottomLeftRadius = config.cornerRadius.bottomLeft.value + 'px';
				}
				if (config.cornerRadius.bottomRight.value > 0) {
					element.style.borderBottomRightRadius = config.cornerRadius.bottomRight.value + 'px';
				}
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_BORDER): {
				let config = readStructAtAddress(renderCommand.config.value, borderConfigDefinition);
				let configMemory = new Uint8Array(memoryDataView.buffer.slice(renderCommand.config.value, renderCommand.config.value + config.__size));
				if (!dirty && !MemoryIsDifferent(configMemory, elementData.previousMemoryConfig, config.__size)) {
					break;
				}
				elementData.previousMemoryConfig = configMemory;
				if (config.left.width.value > 0) {
					let color = config.left.color;
					element.style.borderLeft = `${config.left.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
				}
				if (config.right.width.value > 0) {
					let color = config.right.color;
					element.style.borderRight = `${config.right.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
				}
				if (config.top.width.value > 0) {
					let color = config.top.color;
					element.style.borderTop = `${config.top.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
				}
				if (config.bottom.width.value > 0) {
					let color = config.bottom.color;
					element.style.borderBottom = `${config.bottom.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
				}
				if (config.cornerRadius.topLeft.value > 0) {
					element.style.borderTopLeftRadius = config.cornerRadius.topLeft.value + 'px';
				}
				if (config.cornerRadius.topRight.value > 0) {
					element.style.borderTopRightRadius = config.cornerRadius.topRight.value + 'px';
				}
				if (config.cornerRadius.bottomLeft.value > 0) {
					element.style.borderBottomLeftRadius = config.cornerRadius.bottomLeft.value + 'px';
				}
				if (config.cornerRadius.bottomRight.value > 0) {
					element.style.borderBottomRightRadius = config.cornerRadius.bottomRight.value + 'px';
				}
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_TEXT): {
				let config = readStructAtAddress(renderCommand.config.value, textConfigDefinition);
				let configMemory = new Uint8Array(memoryDataView.buffer.slice(renderCommand.config.value, renderCommand.config.value + config.__size));
				let textContents = renderCommand.text;
				let stringContents = new Uint8Array(memoryDataView.buffer.slice(textContents.chars.value, textContents.chars.value + textContents.length.value));
				if (MemoryIsDifferent(configMemory, elementData.previousMemoryConfig, config.__size)) {
					element.className = 'text';
					let textColor = config.textColor;
					let fontSize = Math.round(config.fontSize.value * GLOBAL_FONT_SCALING_FACTOR);
					element.style.color = `rgba(${textColor.r.value}, ${textColor.g.value}, ${textColor.b.value}, ${textColor.a.value})`;
					element.style.fontFamily = fontsById[config.fontId.value];
					element.style.fontSize = fontSize + 'px';
					element.style.pointerEvents = config.disablePointerEvents.value ? 'none' : 'all';
					elementData.previousMemoryConfig = configMemory;
				}
				if (stringContents.length !== elementData.previousMemoryText.length || MemoryIsDifferent(stringContents, elementData.previousMemoryText, stringContents.length)) {
					element.innerHTML = textDecoder.decode(stringContents);
				}
				elementData.previousMemoryText = stringContents;
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_SCISSOR_START): {
				scissorStack.push({ nextAllocation: { x: renderCommand.boundingBox.x.value, y: renderCommand.boundingBox.y.value }, element, nextElementIndex: 0 });
				let config = readStructAtAddress(renderCommand.config.value, scrollConfigDefinition);
				if (config.horizontal.value) {
					element.style.overflowX = 'scroll';
					element.style.pointerEvents = 'auto';
				}
				if (config.vertical.value) {
					element.style.overflowY = 'scroll';
					element.style.pointerEvents = 'auto';
				}
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_SCISSOR_END): {
				scissorStack.splice(scissorStack.length - 1, 1);
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_IMAGE): {
				let config = readStructAtAddress(renderCommand.config.value, imageConfigDefinition);
				let srcContents = new Uint8Array(memoryDataView.buffer.slice(config.sourceURL.chars.value, config.sourceURL.chars.value + config.sourceURL.length.value));
				if (srcContents.length !== elementData.previousMemoryText.length || MemoryIsDifferent(srcContents, elementData.previousMemoryText, srcContents.length)) {
					element.src = textDecoder.decode(srcContents);
				}
				elementData.previousMemoryText = srcContents;
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_CUSTOM): break;
		}
	}

	for (const key of Object.keys(elementCache)) {
		if (elementCache[key].exists) {
			elementCache[key].exists = false;
		} else {
			elementCache[key].element.remove();
			delete elementCache[key];
		}
	}
}
*/
function renderLoopCanvas() {
// Note: Rendering to canvas needs to be scaled up by window.devicePixelRatio in both width and height.
// e.g. if we're working on a device where devicePixelRatio is 2, we need to render
// everything at width^2 x height^2 resolution, then scale back down with css to get the correct pixel density.
	let capacity = memoryDataView.getUint32(scratchSpaceAddress, true);
	let length = memoryDataView.getUint32(scratchSpaceAddress + 4, true);
	let arrayOffset = memoryDataView.getUint32(scratchSpaceAddress + 8, true);
	window.canvasRoot.width = window.innerWidth * window.devicePixelRatio;
	window.canvasRoot.height = window.innerHeight * window.devicePixelRatio;
	window.canvasRoot.style.width = window.innerWidth + 'px';
	window.canvasRoot.style.height = window.innerHeight + 'px';
	let ctx = window.canvasContext;
	let scale = window.devicePixelRatio;
	for (let i = 0; i < length; i++, arrayOffset += renderCommandSize) {
		let renderCommand = readStructAtAddress(arrayOffset, renderCommandDefinition);
		let boundingBox = renderCommand.boundingBox;
		
		// note: commandType is packed to uint8_t and has 3 garbage bytes of padding
		switch(renderCommand.commandType.value & 0xff) {
			case (CLAY_RENDER_COMMAND_TYPE_NONE): {
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_RECTANGLE): {
				let config = readStructAtAddress(renderCommand.config.value, rectangleConfigDefinition);
				let color = config.color;
				ctx.beginPath();
				window.canvasContext.fillStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
				window.canvasContext.roundRect(
					boundingBox.x.value * scale, // x
					boundingBox.y.value * scale, // y
					boundingBox.width.value * scale, // width
					boundingBox.height.value * scale,
					[config.cornerRadius.topLeft.value * scale, config.cornerRadius.topRight.value * scale, config.cornerRadius.bottomRight.value * scale, config.cornerRadius.bottomLeft.value * scale]) // height;
				ctx.fill();
				ctx.closePath();
				// Handle link clicks
				let linkContents = config.link.length.value > 0 ? textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(config.link.chars.value, config.link.chars.value + config.link.length.value))) : 0;
				memoryDataView.setUint32(0, renderCommand.id.value, true);
				if (linkContents.length > 0 && (window.mouseDownThisFrame || window.touchDown) && instance.exports.Clay_PointerOver(0)) {
					window.location.href = linkContents;
				}
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_BORDER): {
				let config = readStructAtAddress(renderCommand.config.value, borderConfigDefinition);
				ctx.beginPath();
				ctx.moveTo(boundingBox.x.value * scale, boundingBox.y.value * scale);
				// Top Left Corner
				if (config.cornerRadius.topLeft.value > 0) {
					let lineWidth = config.top.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.moveTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale);
					let color = config.top.color;
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.arcTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale, (boundingBox.x.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale, config.cornerRadius.topLeft.value * scale);
					ctx.stroke();
				}
				// Top border
				if (config.top.width.value > 0) {
					let lineWidth = config.top.width.value;
					let halfLineWidth = lineWidth / 2;
					let color = config.top.color;
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.moveTo((boundingBox.x.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale);
					ctx.lineTo((boundingBox.x.value + boundingBox.width.value - config.cornerRadius.topRight.value - halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale);
					ctx.stroke();
				}
				// Top Right Corner
				if (config.cornerRadius.topRight.value > 0) {
					let lineWidth = config.top.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.moveTo((boundingBox.x.value + boundingBox.width.value - config.cornerRadius.topRight.value - halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale);
					let color = config.top.color;
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.arcTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale, (boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.topRight.value + halfLineWidth) * scale, config.cornerRadius.topRight.value * scale);
					ctx.stroke();
				}
				// Right border
				if (config.right.width.value > 0) {
					let color = config.right.color;
					let lineWidth = config.right.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.moveTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.topRight.value + halfLineWidth) * scale);
					ctx.lineTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.topRight.value - halfLineWidth) * scale);
					ctx.stroke();
				}
				// Bottom Right Corner
				if (config.cornerRadius.bottomRight.value > 0) {
					let color = config.top.color;
					let lineWidth = config.top.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.moveTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale);
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.arcTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale, (boundingBox.x.value + boundingBox.width.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale, config.cornerRadius.bottomRight.value * scale);
					ctx.stroke();
				}
				// Bottom Border
				if (config.bottom.width.value > 0) {
					let color = config.bottom.color;
					let lineWidth = config.bottom.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.moveTo((boundingBox.x.value + config.cornerRadius.bottomLeft.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale);
					ctx.lineTo((boundingBox.x.value + boundingBox.width.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale);
					ctx.stroke();
				}
				// Bottom Left Corner
				if (config.cornerRadius.bottomLeft.value > 0) {
					let color = config.bottom.color;
					let lineWidth = config.bottom.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.moveTo((boundingBox.x.value + config.cornerRadius.bottomLeft.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale);
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.arcTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale, (boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomLeft.value - halfLineWidth) * scale, config.cornerRadius.bottomLeft.value * scale);
					ctx.stroke();
				}
				// Left Border
				if (config.left.width.value > 0) {
					let color = config.left.color;
					let lineWidth = config.left.width.value;
					let halfLineWidth = lineWidth / 2;
					ctx.lineWidth = lineWidth * scale;
					ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
					ctx.moveTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomLeft.value - halfLineWidth) * scale);
					ctx.lineTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.bottomRight.value + halfLineWidth) * scale);
					ctx.stroke();
				}
				ctx.closePath();
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_TEXT): {
				let config = readStructAtAddress(renderCommand.config.value, textConfigDefinition);
				let textContents = renderCommand.text;
				let stringContents = new Uint8Array(memoryDataView.buffer.slice(textContents.chars.value, textContents.chars.value + textContents.length.value));
				let fontSize = config.fontSize.value * GLOBAL_FONT_SCALING_FACTOR * scale;
				ctx.font = `${fontSize}px ${fontsById[config.fontId.value]}`;
				let color = config.textColor;
				ctx.textBaseline = 'middle';
				ctx.fillStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
				ctx.fillText(textDecoder.decode(stringContents), boundingBox.x.value * scale, (boundingBox.y.value + boundingBox.height.value / 2 + 1) * scale);
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_SCISSOR_START): {
				window.canvasContext.save();
				window.canvasContext.beginPath();
				window.canvasContext.rect(boundingBox.x.value * scale, boundingBox.y.value * scale, boundingBox.width.value * scale, boundingBox.height.value * scale);
				window.canvasContext.clip();
				window.canvasContext.closePath();
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_SCISSOR_END): {
				window.canvasContext.restore();
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_IMAGE): {
				let config = readStructAtAddress(renderCommand.config.value, imageConfigDefinition);
				let src = textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(config.sourceURL.chars.value, config.sourceURL.chars.value + config.sourceURL.length.value)));
				if (!imageCache[src]) {
					imageCache[src] = {
						image: new Image(),
						loaded: false,
					}
					imageCache[src].image.onload = () => imageCache[src].loaded = true;
					imageCache[src].image.src = src;
				} else if (imageCache[src].loaded) {
					if (config.subrectangle.width.value * config.subrectangle.height.value == 0) {
						ctx.drawImage(imageCache[src].image,
							boundingBox.x.value * scale,
							boundingBox.y.value * scale,
							boundingBox.width.value * scale,
							boundingBox.height.value * scale);
					} else {
						ctx.drawImage(imageCache[src].image,
							config.subrectangle.x.value,
							config.subrectangle.y.value,
							config.subrectangle.width.value,
							config.subrectangle.height.value,
							boundingBox.x.value * scale,
							boundingBox.y.value * scale,
							boundingBox.width.value * scale,
							boundingBox.height.value * scale);
					}
				}
				break;
			}
			case (CLAY_RENDER_COMMAND_TYPE_CUSTOM): break;
		}
	}
}

function renderLoop(currentTime) {
	const elapsed = currentTime - previousFrameTime;
	previousFrameTime = currentTime;
	/*
	let activeRendererIndex = memoryDataView.getUint32(instance.exports.ACTIVE_RENDERER_INDEX.value, true);
	if (activeRendererIndex === 0) {
		instance.exports.UpdateDrawFrame(scratchSpaceAddress, window.innerWidth, window.innerHeight, 0, 0, window.mousePositionXThisFrame, window.mousePositionYThisFrame, window.touchDown, window.mouseDown, 0, 0, window.dKeyPressedThisFrame, elapsed / 1000);
	} else {
		instance.exports.UpdateDrawFrame(scratchSpaceAddress, window.innerWidth, window.innerHeight, window.mouseWheelXThisFrame, window.mouseWheelYThisFrame, window.mousePositionXThisFrame, window.mousePositionYThisFrame, window.touchDown, window.mouseDown, window.arrowKeyDownPressedThisFrame, window.arrowKeyUpPressedThisFrame, window.dKeyPressedThisFrame, elapsed / 1000);
	}
	let rendererChanged = activeRendererIndex !== window.previousActiveRendererIndex;
	switch (activeRendererIndex) {
		case 0: {
			renderLoopHTML();
			if (rendererChanged) {
				window.htmlRoot.style.display = 'block';
				window.canvasRoot.style.display = 'none';
			}
			break;
		}
		case 1: {
			renderLoopCanvas();
			if (rendererChanged) {
				window.htmlRoot.style.display = 'none';
				window.canvasRoot.style.display = 'block';
			}
			break;
		}
	}
	window.previousActiveRendererIndex = activeRendererIndex;
	*/
	instance.exports.UpdateDrawFrame(scratchSpaceAddress, window.innerWidth, window.innerHeight, window.mouseWheelXThisFrame, window.mouseWheelYThisFrame, window.mousePositionXThisFrame, window.mousePositionYThisFrame, window.touchDown, window.mouseDown, window.arrowKeyDownPressedThisFrame, window.arrowKeyUpPressedThisFrame, window.dKeyPressedThisFrame, elapsed / 1000);
	renderLoopCanvas();
	requestAnimationFrame(renderLoop);
	window.mouseDownThisFrame = false;
	window.arrowKeyUpPressedThisFrame = false;
	window.arrowKeyDownPressedThisFrame = false;
	window.dKeyPressedThisFrame = false;
}
init();
