
function SentenceTree(svgElement, inputTokens, options)
{
	var object = {

		originX: options && options.originX ? options.originX : 0,
		originY: options && options.originY ? options.originY : 0,
		options: options,
		
		generate: function (inputTokens) {

			// datu sagatavošana
			if(!inputTokens)
			{
				this.tokens = [];
				return;
			}

			var tokens = [];

			this.element.selectAll('*').remove();

			tokens.push({
				name: '*',
				depth: 0,
				index: 0,
				children: []
			});

			// augment with children
			for(var i in inputTokens)
			{
				var inputToken = inputTokens[i];
				var token = {};
				token.name = inputToken.word;
				token.parentIndex = inputToken.parentIndex;
				// for(var k in inputToken) token[k] = inputToken[k];
				token.children = [];
				// if(i == 0)
				// 	token.depth = 0;
				token.index = tokens.length;
				tokens.push(token);
			}

			// fill children
			for(var i in tokens)
			{
				var token = tokens[i];
				// console.log(token.parentIndex);
				if(token.parentIndex !== undefined && token.parentIndex !== -1)
					tokens[token.parentIndex].children.push(token);
			}

			var done = false;
			while(!done)
			{
				done = true;
				for(var i in tokens)
				{
					var token = tokens[i];
					if(token.depth === undefined && token.parentIndex !== undefined && token.parentIndex !== -1)
					{
						if(tokens[token.parentIndex].depth !== undefined)
							token.depth = tokens[token.parentIndex].depth+1;
						else
							done = false;
					}
				}
			}

			this.tokens = tokens;

			// koka ģenerēšana

			var rectStrokeWidth = 2;
			var lineStrokeWidth = 3;
			var midlineHeight = 7;

			// var baseX = this.originX;
			// var baseY = this.originY;

			var baseX = 10;
			var baseY = 10;
			var distY = 2*midlineHeight+lineStrokeWidth;
			var distX = rectStrokeWidth + 3;
			var nextX = baseX;
			var fontSize = 20;
			var padding = { top: 2, bottom: 3, left: 4, right: 6 };
			// kompensē rāmja biezumu
			for(var k in padding)
				padding[k] += rectStrokeWidth/2;
			var height = fontSize+padding.top+padding.bottom;


			var maxDepth = 0;
			for(var i in tokens)
			{
				var token = tokens[i];
				if(token.depth > maxDepth)
					maxDepth = token.depth;
			}

			this.element.selectAll('g').data(tokens).enter().append('g').each(function (token, i) {
				var g = d3.select(this);
				// console.log(this, arguments);
				var width = 30;
				var x = nextX;
				var y = baseY + (height+distY)*(token.depth === undefined ? 0 : token.depth);
				var text = g.append('text')
					.attr('x', x+padding.left)
					.attr('y', y+fontSize+padding.top)
					.attr('transform', 'scale(1, 1)')
					.attr('font-size', fontSize)
					// .text(inputTokens[token.index].name);
					.text(token.name);
					// .style('stroke', 'black');
					// console.log(text);
				height = text[0][0].getBBox().height+padding.top+padding.bottom;
				width = text[0][0].getBBox().width+padding.left+padding.right;
				// console.log(height, width);
				
				g.append('rect')
					.attr('x', x)
					.attr('y', y)
					.attr('width', width)
					.attr('height', height)
					.style('stroke', 'black')
					.style('stroke-width', rectStrokeWidth)
					.style('fill', 'none');
				nextX += width+distX;

				token.x = x;
				token.y = y;
				token.width = width;
				token.height = height;

				x += width/2;
				if(token.depth > 0)
				{
					g.append('line')
						.attr('x1', x).attr('y1', y-midlineHeight-lineStrokeWidth).attr('x2', x).attr('y2', y)
						.attr('stroke', 'black').attr('stroke-width', lineStrokeWidth);
				}
				if(token.depth < maxDepth && token.children.length > 0)
				{
					g.append('line')
						.attr('x1', x).attr('y1', y+height).attr('x2', x).attr('y2', y+height+midlineHeight+lineStrokeWidth)
						.attr('stroke', 'black').attr('stroke-width', lineStrokeWidth);
				}
			});

			function drawChildrenConnectorLine(token) {

				var first = token, last = token;

				for(var i in token.children)
				{
					var child = token.children[i];

					if(first.index > child.index)
						first = child;
					if(last.index < child.index)
						last = child;
				}

				if(first && last)
				{
					var line = this.element.append('line');

					var y = baseY + (height+distY)*(token.depth+1) - distY/2;
					line.attr('stroke', 'black').attr('stroke-width', lineStrokeWidth);
					line.attr('y1', y).attr('y2', y);
					line.attr('x1', first.x+first.width/2);
					line.attr('x2', last.x+last.width/2);
				}

				for(var i in token.children)
					drawChildrenConnectorLine.call(this, token.children[i]);
			}

			drawChildrenConnectorLine.call(this, tokens[0]);

			return this;
		}
	};

	object.element = svgElement.append('g');
	// object.originX = 0;
	// object.originY = 0;

	// var offsetX, offsetY;

	// var drag = d3.behavior.drag()
	// 	.on("dragstart", function () {
	// 		var rect = this.getBoundingClientRect();
	// 		offsetX = d3.event.sourceEvent.clientX - rect.left - this.clientLeft - object.originX;
	// 		offsetY = d3.event.sourceEvent.clientY - rect.top - this.clientTop - object.originY;
	// 	})
	// 	.on("dragend", function () {
	// 		var rect = this.getBoundingClientRect();
	// 		object.originX = d3.event.sourceEvent.clientX - rect.left - this.clientLeft - offsetX;
	// 		object.originY = d3.event.sourceEvent.clientY - rect.top - this.clientTop - offsetY;
	// 	})
	// 	.on("drag", function (d) {
	// 		// console.log('scale', d3.event.scale);
	// 		object.element.attr('transform', function (d) {
	// 			return 'translate('+(d3.event.x-offsetX)+','+(d3.event.y-offsetY)+')';
	// 		});
	// 	});

	// $(svgElement[0][0]).bind('DOMMouseScroll mousewheel', function (event) {
	// 	var delta = event.originalEvent.wheelDelta;
	// 	// var delta = event.wheelDelta || -event.detail;
	// 	// console.log('mousewheel', event, delta);
	// 	svgElement.scale
	// });
	
	var zoom = d3.behavior.zoom()
		.scaleExtent([0.3, 3])
		.on("zoom", function () {
			object.element.attr('transform', 'translate('+d3.event.translate+') scale('+d3.event.scale+')');
		});

	svgElement.call(zoom);
	// object.element.call(zoom);

	// object.element.call(drag);
	// svgElement.call(drag);

	object.generate(inputTokens);

	return object;
}


