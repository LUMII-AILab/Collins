"use strict";

function JITTree(element)
{
	function convert(tokens) {

		var tree = [];

		var root = {
			id: 0,
			name: '*',
			data: {
				lemma: '*',
				tag: 'R',
				parentIndex: -1
			},
			children: []
		};

		tree.push(root);

		if(!tokens)
			return root;

		for(var i in tokens)
		{
			var token = tokens[i];
			tree.push({
				id: token.index,
				name: token.word,
				data: {
					lemma: token.lemma,
					tag: token.tag,
					parentIndex: token.parentIndex
				},
				children: []
			});
		}

		for(var i in tree)
		{
			var child = tree[i];
			if(child.data.parentIndex === -1)
				continue;

			tree[child.data.parentIndex].children.push(child);
		}

		return root;
	}

	function create() {

		tree = new $jit.ST({
			injectInto: element,
			orientation: 'top',
			//Add node/edge styles
			Node: {
				overridable: false,  
				// type: 'piechart',
				// type: 'rectangle',
				width: 90,
				height: 36,
				// autoWidth: false,
				// autoHeight: false,
				// color: '#ccb',
				// color: '#',
				CanvasStyles: {
					// fillStyle: '#daa',
					fillStyle: '#ffffff',
					// strokeStyle: '#fcc',
					lineWidth: 2
				}
			},
			Edge: {
				// color: '#999',
				color: '#000000',
				// type: 'quadratic:begin'
				// type: 'line'
			},
			// Label: {
			// 	type: 'SVG',
			// 	color: 'red'
			// },
			//Parent-children distance
			levelDistance: 15,
			levelsToShow: 20,
			duration: 0,
			constrained: false,
			Navigation: {
				enable: true,
				panning: true,
			},
			//Add styles to node labels on label creation
			onCreateLabel: function(domElement, node){
				//add some styles to the node label
				var style = domElement.style;
				domElement.id = node.id;
				style.border = '1px solid black';
				style.color = 'black';
				style.fontSize = '15px';
				style.textAlign = 'left';
				style.width = "89px";
				style.height = "36x";
				// style.height = "24px";
				// style.paddingTop = "22px";
				style.cursor = 'pointer';
				var name = node.name;
				var tag = node.data.tag;
				var red = '';
				if(node.data.reduction)
				{
					parts = node.data.reduction.split('(');
					if(parts.length == 2)
						name = parts[1].split(')')[0];
					tag = parts[0];
					red = '<i>red.: </i>';
				}
				domElement.innerHTML = red + '<b>' + name + '<br/><i>' + (tag ? tag : '&nbsp') + '</i></b>';
				// domElement.onclick = function() {
				//   module.st.onClick(node.id, {
				// 		Move: {
				// 			offsetY: 90
				// 			// offsetY: -90
				// 		}
				// 	});  
				// };
			},

			// **** currently not used ****

			//Add the name of the node in the correponding label
			//and a click handler to move the graph.
			//This method is called once, on label creation.
			onCreateLabel0: function(domElement, node){
				domElement.firstChild
				  .appendChild(document
					.createTextNode(node.name));
				domElement.onclick = function(){
					rgraph.onClick(node.id, {
					  hideLabels: false
					});
				};
			},
			//Change some label dom properties.
			//This method is called each time a label is plotted.
			onPlaceLabel0: function(domElement, node){
				var bb = domElement.getBBox();
				domElement.setAttribute('transform', 'translate('+domElement.getAttribute('x')+','+domElement.getAttribute('y')+')');
				return;
				if(bb) {
				  //center the label
				  var x = domElement.getAttribute('x');
				  var y = domElement.getAttribute('y');
				  //get polar coordinates
				  var p = node.pos.getp(true);
				  //get angle in degrees
				  var pi = Math.PI;
				  var cond = (p.theta > pi/2 && p.theta < 3* pi /2);
				  if(cond) {
					domElement.setAttribute('x', x - bb.width );
					domElement.setAttribute('y', y - bb.height );
				  } else if(node.id == rgraph.root) {
					domElement.setAttribute('x', x - bb.width/2); 
				  }
				  
				  var thetap =  cond? p.theta + pi : p.theta;
					domElement.setAttribute('transform', 'rotate('
					+ thetap * 360 / (2 * pi) + ' ' + x + ' ' + y + ')');
				}
			}
		});
	}

	function load(data) {

		if(!tree)
			return;

		tree.loadJSON(data);
		tree.compute();
		// nav skaidrs kāpēc, bet bez šī ir dīvaini gļuki
		tree.onClick(tree.root, {
			Move: {
		// 		offsetY: 200
			}
		});

		object.resize(object.width, object.height, true);
	}

	var tree;

	var object = {

		data: undefined,

		show: function () {
			if(!tree)
				create();

			if(this.data)
				load(this.data);
		},

		resize: function (width, height, preserveTranslation) {

			if(width === undefined)
				width = this.width;
			else
				this.width = width;

			if(height === undefined)
				height = this.height;
			else
				this.height = height;

			if(!tree)
				return;

			if(!preserveTranslation)
				tree.canvas.resize(width, height);

			var tx = tree.canvas.translateOffsetX;
			var ty = tree.canvas.translateOffsetY;
			tree.canvas.resize(width, height);
			tree.canvas.translate(tx, ty);
		},

		load: function (tokens) {

			this.data = convert(tokens);

			load(this.data);
		}
	};

	// load default empty set (lai izvairītos no erroriem)
	object.load([]);

	return object;
}
