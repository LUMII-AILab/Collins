

function DirectedForceTree(svg) {

	function tick(e) {
		// Push sources up and targets down to form a weak tree.
		var k = 6 * e.alpha;
		object.data.links.forEach(function (d, i) {
			d.source.y -= k;
			d.target.y += k;
		});

		object.node.attr('cx', function (d) { return d.x; })
			.attr('cy', function (d) { return d.y; });

		object.label.attr('x', function (d) { return d.x+5; })
			.attr('y', function (d) { return d.y; });

		object.link.attr('x1', function (d) { return d.source.x; })
			.attr('y1', function (d) { return d.source.y; })
			.attr('x2', function (d) { return d.target.x; })
			.attr('y2', function (d) { return d.target.y; });
	}

	function convert(tokens) {

		function addRoot() {
			data.nodes.push({
				index: 0,
				// name: '*',
				name: '* (ROOT)',
				parentIndex: -1,
				root: true
			});
			return data;
		}

		var data = {
			nodes: [],
			links: []
		};

		if(!tokens || !tokens.length)
			return addRoot();

		if(tokens[0].index !== 0)
			addRoot();

		// TODO: saskaitīt, cik katram mezglam ir apakšmezglu un pamatojoties uz to mainīt mezglu lādiņus

		for(var i in tokens)
		{
			var token = tokens[i];
			var index = data.nodes.length;

			data.nodes.push({
				index: index,
				name: token.word,
				parentIndex: token.parentIndex,
				root: index === 0
			});

			if(token.parentIndex != undefined)
				data.links.push({
					source: token.parentIndex,
					target: index
				});
		}

		return data;
	}

	var object = {
		// width: $(svg).width(),
		// height: $(svg).height(),
		width: svg.attr('width'),
		height: svg.attr('height'),
		data: {
			nodes: [],
			links: []
		},

		resize: function (width, height) {
			this.width = width;
			this.height = height;
			force.size([width, height]);
		},

		load: function (tokens) {

			force.stop();

			if(this.label)
				this.label.remove();

			if(this.link)
				this.link.remove();

			if(this.node)
				this.node.remove();

			this.data = convert(tokens);

			var root = this.data.nodes[0];
			root.fixed = true;
			root.x = this.width / 2;
			root.y = this.height / 10;

			this.link = g.selectAll('line').data(this.data.links).enter().append('svg:line')
				.style('stroke', function (d) { return d3.rgb('black'); });

			this.node = g.selectAll('circle').data(this.data.nodes).enter().append('svg:circle')
				.attr('r', function (d) { return d.root ? 5 : 2; })
				.style('fill', function (d) { return d3.rgb('black'); })
				.style('stroke', function (d) { return d3.rgb('black'); })
				.call(force.drag);

			this.label = g.selectAll('text').data(this.data.nodes).enter().append('svg:text')
				.text(function (d) { return d.name; }).call(force.drag);

			force.nodes(this.data.nodes).links(this.data.links).start();
		}
	};

	var g = svg.append('g');

	var force = d3.layout.force()
		.gravity(0.3)
		// .charge(-300)
		.charge(-500)
		.linkDistance(40)
		.size([object.width, object.height]);

	force.on('tick', tick);

	var zoom = d3.behavior.zoom()
		.scaleExtent([0.3, 3])
		.on("zoom", function () {
			g.attr('transform', 'translate('+d3.event.translate+') scale('+d3.event.scale+')');
		});

	svg.call(zoom);

	return object;
}

