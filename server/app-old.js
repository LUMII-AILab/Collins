"use strict";

// var app = angular.module('app', ['ngResource', 'ui', 'ngDragDrop', 'ui.bootstrap']);
var app = angular.module('app', ['ngResource', 'ui', 'ui.bootstrap']);


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


app.directive('editable', function ($parse) {
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
					// var val = edit.val().replace('"', '\\"');
					var val = edit.val();
					if(element.hasClass('integer'))
					{
						// http://www.texotela.co.uk/code/jquery/numeric/
						// https://github.com/SamWM/jQuery-Plugins/blob/master/numeric/jquery.numeric.js
						val = parseInt(val.replace(/[^-0-9]/g, ''));
						if(isNaN(val))
							val = 0;
					}
					// old way (bad)
					// console.log(attrs.ngBind + '="' + edit.val().replace('"', '\\"') + '"');
					// scope.$apply(attrs.ngBind + '="' + val + '"');
					// https://groups.google.com/forum/?fromgroups=#!searchin/angular/directive$20two$20binding$20parent$20without$20isolate/angular/p6TkKUmXOhA/ToqD2lK3bawJ
					// https://groups.google.com/forum/#!topic/angular/yI-iMUFBU6s/discussion
					// http://docs.angularjs.org/api/ng.$parse
					scope.$apply(function () {
						$parse(attrs.ngBind).assign(scope, val);
					});
					
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

app.directive('focus', function ($timeout, $parse) {
	return {
		restrict: 'A',
		link: function (scope, element, attrs) {
			// var enable = scope.$eval(attrs.focus);
			if(attrs.focus === undefined)
				$timeout(function () {
					element[0].focus();
				}, 100);
			else
			{
				// var preventEvent = false;
				scope.$watch(attrs.focus, function (value) {
					// if(preventEvent) return;
					$timeout(function () {
						if(value) element[0].focus();
						else element[0].blur();
					}, 0);
					// preventEvent = false;
				});
				element.focusin(function () {
					if(scope.$root.$$phase) return;
					scope.$apply(function () {
						// preventEvent = true;
						$parse(attrs.focus).assign(scope, true);
					});
				});
				element.focusout(function () {
					if(scope.$root.$$phase) return;
					scope.$apply(function () {
						// preventEvent = true;
						$parse(attrs.focus).assign(scope, false);
					});
				});
			}
		}
	};
});

app.directive('selectOnFocus', function () {
	return function (scope, element, attrs) {
		element.focusin(function () {
			element.select();
		});
	};
});

// http://job-blog.bullgare.ru/2013/01/%D0%B4%D0%B8%D1%80%D0%B5%D0%BA%D1%82%D0%B8%D0%B2%D1%8B-%D0%B4%D0%BB%D1%8F-angular-js/
app.directive('ngBlur', function (/* $parse */) {
	return {
		restrict: 'A',
		link: function postLink(scope, element, attrs) {
			element.bind('blur', function () {
				scope.$apply(function ($scope) {
					// $parse(attrs.ngBlur)(scope, {value: element.val()});	// manuāli
					scope.$eval(attrs.ngBlur, {value: element.val()});
				});
				// scope.$apply(attrs.ngBlur)
			});
		}
	};
});

app.directive('spin', function () {
	return {
		restrict: 'E',
		replace: true,
		require: '?ngModel',
		transclude: true,
		template: '<div style="position: relative;"></div>',
		link: function (scope, element, attrs, ngModel) {

			var opts = {
				lines: 13, // The number of lines to draw
				length: 5, // The length of each line
				width: 2, // The line thickness
				radius: 3, // The radius of the inner circle
				corners: 1, // Corner roundness (0..1)
				rotate: 0, // The rotation offset
				direction: 1, // 1: clockwise, -1: counterclockwise
				color: '#000', // #rgb or #rrggbb
				speed: 1, // Rounds per second
				trail: 60, // Afterglow percentage
				shadow: false, // Whether to render a shadow
				hwaccel: false, // Whether to use hardware acceleration
				className: 'spinner', // The CSS class to assign to the spinner
				zIndex: 2e9, // The z-index (defaults to 2000000000)
				top: 'auto', // Top position relative to parent in px
				left: 'auto' // Left position relative to parent in px
			};

			// override options
			angular.extend(opts, scope.$eval(attrs.options));

			var spinner = new Spinner(opts);
			if(!ngModel)
			{
				spinner.spin(element[0]);
				return;
			}

			scope.$watch(attrs.ngModel, function (value) {
				if(value)
					spinner.spin(element[0]);
				else
					spinner.stop();
			});
		}
	};
});

app.controller('AppController', function ($scope, $location, $timeout, $http) {
});

app.controller('MultiController', function ($scope, $location, $timeout, $http) {
	$scope.trees = [];
	$scope.selected = undefined;
	$scope.select = function (item) { $scope.selected = item; }


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

	$scope.load = function(conll) {
		if(!conll || conll.length == 0)
			return;

		$scope.$apply(function () {

		// var lines = conll.match(/[^\r\n]+/g);
		var lines = conll.split(/\n/);
		// console.log(lines);
		var output = [];
		var words = [];
		for(var i in lines)
		{
			var line = lines[i].trim();
			// console.log(line);
			if(line.length == 0)
			{
				// console.log("!!!!!");
				if(output.length > 0)
				{
					// console.log('push:', output);
					$scope.trees.push({data: output, text: words.join(' ')});
				}
				words = [];	
				output = [];
				continue;
			}

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
			words.push(parts[1]);
		}
		// return output;
		});
	};

	function genCoNLL(data) {
		var result = '';
		for(var i in data)
		{
			var row = data[i];
			result += [row.index, row.word, row.lemma, row.tag0, row.tag, row.features, row.parentIndex].join('\t') + '\n';
		}
		return result;
	};

	$scope.downloadCoNLL = function () {
		var conll = [];
		for(var i in $scope.trees)
		{
			conll.push(genCoNLL($scope.trees[i].data));
		}
		showSave(conll.join('\n'), 'output.conll', 'application/json');
	};

	$scope.parse = function () {
		$scope.state.inProgress = true;
		var conll = genCoNLL($scope.selected.data);
		// console.log('parse sent:', conll);
		$http.post('rest/parse', conll).success(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			// console.log('success:', data);
			// $scope.outputCoNLL.raw = data;
			$scope.selected.data = parseCoNLL(data);
		}).error(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			console.log('error:', data);
		});
	};

	$scope.state = {inProgress: false};

	$scope.dropClass = '';

	//============== DRAG & DROP =============
    // source for drag&drop: http://www.webappers.com/2011/09/28/drag-drop-file-upload-with-html5-javascript/
    var dropbox = document.getElementById("dropbox");
    // $scope.dropText = 'Drop files here...'

    // init event handlers
    function dragEnterLeave(event) {
        event.stopPropagation();
        event.preventDefault();
        $scope.$apply(function() {
            $scope.dropClass = '';
        });
    };
    dropbox.addEventListener("dragenter", dragEnterLeave, false);
    dropbox.addEventListener("dragleave", dragEnterLeave, false);
    dropbox.addEventListener("dragover", function(event) {
        event.stopPropagation();
        event.preventDefault();
        // var clazz = 'not-available'
        var ok = event.dataTransfer && event.dataTransfer.types && event.dataTransfer.types.indexOf('Files') >= 0;
        $scope.$apply(function(){
        //     $scope.dropText = ok ? 'Drop files here...' : 'Only files are allowed!'
            $scope.dropClass = ok ? 'over' : 'not-available';
			// console.log($scope.dropClass);
        });
    }, false);
    dropbox.addEventListener("drop", function(event) {
        // console.log('drop event:', JSON.parse(JSON.stringify(event.dataTransfer)))
        event.stopPropagation();
        event.preventDefault();
        $scope.$apply(function() {
            // $scope.dropText = 'Drop files here...'
            $scope.dropClass = '';
        });
        var files = event.dataTransfer.files;
        if(files.length > 0)
		{
			for (var i = 0, f; f = files[i]; i++)
			{
				var reader = new FileReader();
				reader.onload = function (event) {
					$scope.load(this.result);
				};

				reader.readAsText(f);
			}
            // $scope.$apply(function(){
            //     $scope.files = []
            //     for (var i = 0; i < files.length; i++)
				// {
            //         $scope.files.push(files[i]);
            //     }
            // });
        }
    }, false);
    //============== DRAG & DROP =============
});

app.controller('SingleController', function ($scope, $location, $timeout, $http) {

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

	$scope.outputDisabled = function () {
		return !$scope.outputCoNLL.data || $scope.outputCoNLL.data.length == 0;
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

	$scope.downloadOutput = function () {
		$scope.outputCoNLL.raw = genCoNLL($scope.outputCoNLL.data);
		showSave($scope.outputCoNLL.raw, 'download.conll', 'application/json');
		// selectTab('output');
	};

	$scope.acceptOutputRaw = function () {
		$scope.outputCoNLL.data = parseCoNLL($scope.outputCoNLL.raw);
		// selectTab('output');
	};

	$scope.getCoNLL = function () {
		$scope.state.inProgress = true;
		selectTab('input');
		// $scope.inputCoNLL.editable = true;
		$http.post('rest/conllize', $scope.sentence).success(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			// console.log('success:', data);
			$scope.inputCoNLL.raw = data;
			$scope.inputCoNLL.data = parseCoNLL(data);
		}).error(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			console.log('error:', data);
		});
	};

	$scope.parse = function () {
		$scope.state.inProgress = true;
		// $scope.inputCoNLL.editable = false;
		var conll = genCoNLL($scope.inputCoNLL.data);
		// console.log('parse sent:', conll);
		selectTab('output');
		$http.post('rest/parse', conll).success(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			// console.log('success:', data);
			$scope.outputCoNLL.raw = data;
			$scope.outputCoNLL.data = parseCoNLL(data);
		}).error(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			console.log('error:', data);
		});
	};

	$scope.$watch('selectedTab.tree', function (value) {

		if(!value) return;

		var data = createTree($scope.outputCoNLL.data);

		// $http.post('rest/conll2json', $scope.outputCoNLL.raw).success(function (data, status, headers, config) {
		// $http.get('pretty.json').success(function (data, status, headers, config) {
			// data = data[1].root;
			// console.log('success:', data);
			// console.log(data);
			$scope.outputCoNLL.json = data;
			setTree(data);
		// }).error(function (data, status, headers, config) {
		// 	console.log('error:', data);
		// });
	});

	$scope.showTree = function () {
		selectTab('tree');
		$scope.conllToJSON();
	};

	$scope.test = 'abc';

	$scope.drop = function (event) {
		console.log('drop:', event);
	};

	$scope.state = { copyFocus: false, copyVisible: false, pasteFocus: false, pasteVisible: false, inProgress: false };

	$scope.showCopy = function () {
		if(!$scope.state.copyVisible)
		{
			$scope.outputCoNLL.raw = genCoNLL($scope.outputCoNLL.data);
			$scope.state.copyFocus = true;
			$scope.state.copyVisible = true;
		}
	};

	$scope.hideCopy = function () {
		$timeout(function () {
			$scope.state.copyVisible = false;
		}, 100);
	};

	$scope.showPaste = function () {
		if(!$scope.state.pasteVisible)
		{
			$scope.outputCoNLL.raw = genCoNLL($scope.outputCoNLL.data);
			$scope.state.pasteFocus = true;
			$scope.state.pasteVisible = true;
		}
	};

	$scope.hidePaste = function () {
		$timeout(function () {
			$scope.state.pasteVisible = false;
			$scope.inputCoNLL.data = parseCoNLL($scope.inputCoNLL.raw);
		}, 100);
	};


	var createTree = function (data)
	{
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

		for(var i in data)
		{
			var node = data[i];
			tree.push({
				id: node.index,
				name: node.word,
				data: {
					lemma: node.lemma,
					tag: node.tag,
					parentIndex: node.parentIndex
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
	};

	var setTree = function (data) {
		if(!tree)
		{
			$timeout(function () {
				tree = initTree();
				setTree(data);
			}, 300);
		}
		else
		{
			tree.loadJSON(data);
			tree.compute();
			tree.onClick(tree.root, {
				Move: {
					offsetY: 200
				}
			});
		}
	};


	var tree;
	var initTree = function () {
		var tmp = new $jit.ST({
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
		return tmp;
	};
});

