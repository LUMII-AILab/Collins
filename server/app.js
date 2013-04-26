"use strict";

var app = angular.module('app', ['ngResource', 'ui', 'ngDragDrop', 'ui.bootstrap']);


// http://stackoverflow.com/questions/14544741/angularjs-directive-to-stoppropagation/14547223#14547223
// usage: <a ng-click='...' stop-event='click'>'
app.directive('stopEvent', function () {
	return {
		restrict: 'A',
		link: function (scope, element, attrs) {
			element.bind(attrs.stopEvent, function (e) {
				e.stopPropagation();
			});
		}
	};
});

app.directive('dropzone', function () {
	return {
		restrict: 'A',
		scope: {
			fn: '=dropzone',
		},
		transclude: true,
		link: function (scope, element, attrs) {
				  scope.fn();
			if(!attrs.dropzone)
				return;
			element[0].addEventListener('drop', function(event) { // if(event.stopPropagation)
			    event.stopPropagation(); // stops the browser from redirecting.
			    event.preventDefault(); // stops the browser from redirecting.})
				scope.dropzone(event)
			});
		}
	};
});


app.directive('editable', function () {
	return {
		restrict: 'A',
		scope: true,
		// scope: {
		// 	r: '='
		// },
		// require: '?ngModel',
		link: function (scope, element, attrs) {
			element.bind('dblclick', function () {
				if(attrs.editable && !scope.$eval(attrs.editable))
					return;

				var edit = $('<input type="text" class="edit-cell" />');
				edit.appendTo(element);
				edit.val(scope.$eval(attrs.ngBind));
				edit.focus();
				edit.focusout(function () {
					// dirty hack
					var val = edit.val().replace('"', '\\"');
					if(element.hasClass('integer'))
					{
						// http://www.texotela.co.uk/code/jquery/numeric/
						// https://github.com/SamWM/jQuery-Plugins/blob/master/numeric/jquery.numeric.js
						val = parseInt(val.replace(/[^-0-9]/g, ''));
						if(isNaN(val))
							val = 0;
					}
					// console.log(attrs.ngBind + '="' + edit.val().replace('"', '\\"') + '"');
					scope.$apply(attrs.ngBind + '="' + val + '"');
					edit.remove();
				});
				edit.keyup(function (e) {
					if(e.keyCode == 13)
						edit.blur();
					else if(e.keyCode == 27)
					{
						edit.val(scope.$eval(attrs.ngBind));
						edit.blur();
					}
				});
			});
		}
	};
});

app.directive('focus', function ($timeout) {
	return {
		restrict: 'A',
		link: function (scope, element, attrs) {
			var enable = scope.$eval(attrs.focus);
			if(enable === undefined || enable)
				$timeout(function () {
					element[0].focus();
				}, 100)
		}
	};
});

app.controller('AppController', function ($scope, $location, $timeout, $http) {

	function parseCoNLL(conll) {
		if(!conll || conll.length == 0)
			return;

		var lines = conll.match(/[^\r\n]+/g);
		var output = [];
		for(var i in lines)
		{
			var line = lines[i];
			if(line.length == 0)
				continue;

			var parts = line.split('\t');
			output.push({
				index: parseInt(parts[0]),
				word: parts[1],
				lemma: parts[2],
				tag0: parts[3],
				tag: parts[4],
				features: parts[5],
				parentIndex: parseInt(parts[6] && parts[6].length > 0 ? parseInt(parts[6]) : -1)
			});
		}
		return output;
	}

	function genCoNLL(data) {
		var result = '';
		for(var i in data)
		{
			var row = data[i];
			result += [row.index, row.word, row.lemma, row.tag0, row.tag, row.features, row.parentIndex].join('\t') + '\n';
		}
		return result;
	};

	var deselectTabs = function () {
		for(var k in $scope.selectedTab)
			$scope.selectedTab[k] = false;
	};
	var selectTab = function (tab) {
		deselectTabs();
		$scope.selectedTab[tab] = true;
	};

	$scope.selectedTab = { rawInput: false, input: true, output: false, rawOutput: false, tree: false };
	$scope.inputCoNLL = { editable: true };
	$scope.outputCoNLL = { editable: true };
	$scope.sentence = '';

	$scope.acceptInputRaw = function () {
		$scope.inputCoNLL.data = parseCoNLL($scope.inputCoNLL.raw);
		// selectTab('input');
	};

	$scope.acceptInput = function () {
		$scope.inputCoNLL.raw = genCoNLL($scope.inputCoNLL.data);
		// selectTab('input');
	};

	$scope.acceptOutput = function () {
		$scope.outputCoNLL.raw = genCoNLL($scope.outputCoNLL.data);
		// selectTab('output');
	};

	$scope.acceptOutputRaw = function () {
		$scope.outputCoNLL.data = parseCoNLL($scope.outputCoNLL.raw);
		// selectTab('output');
	};

	$scope.getCoNLL = function () {
		selectTab('input');
		// $scope.inputCoNLL.editable = true;
		$http.post('rest/conllize', $scope.sentence).success(function (data, status, headers, config) {
			// console.log('success:', data);
			$scope.inputCoNLL.raw = data;
			$scope.inputCoNLL.data = parseCoNLL(data);
		}).error(function (data, status, headers, config) {
			console.log('error:', data);
		});
	};

	$scope.parse = function () {
		// $scope.inputCoNLL.editable = false;
		var conll = genCoNLL($scope.inputCoNLL.data);
		// console.log(conll);
		selectTab('output');
		$http.post('rest/parse', conll).success(function (data, status, headers, config) {
			// console.log('success:', data);
			$scope.outputCoNLL.raw = data;
			$scope.outputCoNLL.data = parseCoNLL(data);
		}).error(function (data, status, headers, config) {
			console.log('error:', data);
		});
	};

	$scope.conllToJSON = function () {
		$http.post('rest/conll2json', $scope.outputCoNLL.raw).success(function (data, status, headers, config) {
		// $http.get('pretty.json').success(function (data, status, headers, config) {
			// data = data[1].root;
			// console.log('success:', data);
			$scope.outputCoNLL.json = data;
			if(!tree)
			{
				$timeout(function () {
					tree = initTree();
					tree.loadJSON($scope.outputCoNLL.json);
					tree.compute();
					tree.onClick(tree.root, {
						Move: {
							offsetY: 200
						}
					});
				}, 300);
			}
			else
			{
				tree.loadJSON($scope.outputCoNLL.json);
				tree.compute();
				tree.onClick(tree.root, {
					Move: {
						offsetY: 200
					}
				});
			}
		}).error(function (data, status, headers, config) {
			console.log('error:', data);
		});
	};

	$scope.showTree = function () {
		selectTab('tree');
		$scope.conllToJSON();
	};

	$scope.test = 'abc';

	$scope.drop = function (event) {
		console.log('drop:', event);
	};


	var tree;
	var initTree = function () {
		return new $jit.ST({
			injectInto: 'space',
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
			}
		});
	};
});

