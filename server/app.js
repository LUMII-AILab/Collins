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

app.directive('noDrop', function () {
	return {
		restrict: 'A',
		link: function (scope, element, attrs) {

			function prevent(event) {
				event.stopPropagation();
				event.preventDefault();
			}

			element[0].addEventListener('dragenter', prevent, false);
			element[0].addEventListener('dragleave', prevent, false);
			element[0].addEventListener('dragover', prevent, false);
			element[0].addEventListener('drop', prevent, false);
		}
	};
});

app.directive('dropzone', function () {
	return {
		restrict: 'A',
		scope: true,
		link: function (scope, element, attrs) {

			function dragEnterLeave(event) {
				event.stopPropagation();
				event.preventDefault();
				scope.$apply(function() {
					scope.dragState = '';
				});
			}

			element.addClass('dropzone');

			// TODO: portablāku veidu, kā pievienot eventus
			element[0].addEventListener('dragenter', dragEnterLeave, false);
			element[0].addEventListener('dragleave', dragEnterLeave, false);
			element[0].addEventListener('dragover', function (event) {
				event.stopPropagation();
				event.preventDefault();
				// TODO: šito būtu kaut kā jākonfigurē
				var ok = event.dataTransfer && event.dataTransfer.types && event.dataTransfer.types.indexOf('Files') >= 0;
				scope.$apply(function () {
					scope.dragState = ok ? 'over' : 'not-available';
				});
			}, false);
			element[0].addEventListener('drop', function (event) {
				event.stopPropagation();
				event.preventDefault();
				scope.$apply(function () {
					scope.dragState = '';
				});
				// callback
				scope.$eval(attrs.dropzone)(event);
				/*
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
				}
				*/
			}, false);
		}
	};
});

app.directive('dropzone0', function () {
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
		template: '<div class="spin"></div>',
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

app.directive('coref', function () {
	return {
		restrict: 'E',
		transclude: true,
		replace: true,
		require: '?ngModel',
		scope: true,
		template: '<div class="coref"><div class="coref-sidebar"></div><div class="coref-view"></div></div>',
		link: function (scope, element, attrs, ngModel) {

			// TODO: te ir jāpieliek unicode pēdiņas, kas ir ar noteiktu vērsumu
			var noGapBefore = ['.', ',', ':', ';', '!', '?', ')', ']', '}', '%'];
			var noGapAfter = ['(', '[', '{'];
			var quoteSymbols = ["'", '"'];

			function gap(prev, next, quotes)
			{
				if(!prev && quoteSymbols.indexOf(next.lemma) !== -1)
					quotes[next.lemma] = !quotes[next.lemma];
				if(!prev)
					return '';
				if(noGapAfter.indexOf(prev.lemma) !== -1)
					return '';
				if(noGapBefore.indexOf(next.lemma) !== -1)
					return '';
				if(quoteSymbols.indexOf(next.lemma) !== -1)
				{
					var quoted = quotes[next.lemma];
					quotes[next.lemma] = !quotes[next.lemma];
					if(quoted)
						return '';
				}
				if(quoteSymbols.indexOf(prev.lemma) !== -1)
				{
					if(quotes[prev.lemma])
						return '';
				}
				return ' ';
			}

			function fill(data) {

				function selectNamedEntitiesWithID(selectID)
				{
					for(var ID in namedEntityElementsByID)
					{
						var elements = namedEntityElementsByID[ID];
						for(var i in elements)
						{
							var element = elements[i];
							if(selectID === undefined)
							{
								element.removeClass('dimmed');
								continue;
							}

							// console.dir(element[0]);
							// console.log(element[0].dataset.sentenceIndex);
							// console.log(element[0].dataset.tokenIndex);
							// console.log(element[0].dataset.sentenceIndex, element[0].dataset.tokenIndex);
							// console.log(element.dataset.sentenceIndex, element.dataset.tokenIndex);
							// console.log(ID == selectID);
							// if(ID != selectID)
							if(selectID != element[0].dataset.namedEntityId)
								element.addClass('dimmed');
							else
								element.removeClass('dimmed');
						}
					}

					if(hideSentences && selectID !== undefined)
					{
						namedEntitySentenceElements = sentenceElementsByNamedEntityID[selectID];

						for(var i in allSentenceElements)
						{
							var sentenceElement = allSentenceElements[i];
							if(namedEntitySentenceElements.indexOf(sentenceElement) === -1)
							{
								sentenceElement.addClass('hidden');
								sentenceElement.removeClass('highlight');
							}
							else
							{
								sentenceElement.removeClass('hidden');
								sentenceElement.addClass('highlight');
							}
						}
					}
					else
					{
						for(var i in allSentenceElements)
						{
							allSentenceElements[i].removeClass('hidden');
							allSentenceElements[i].removeClass('highlight');
						}
					}
				}

				var hideSentences = true;

				// console.log(element);
				var viewElement = element.children('.coref-view');
				viewElement.empty();

				// deselect named entities
				viewElement.click(function (event) {
					event.stopPropagation();
					selectNamedEntitiesWithID();
				});

				var sidebarElement = element.children('.coref-sidebar');
				sidebarElement.empty();
				sidebarElement.append('Entity Types:<br/>');

				var namedEntityTypes = {'_': 0};
				var nextNamedEntityTypeIndex = 1;
				var namedEntityElementsByID = {};
				var allTokenElements = [];
				var allSentenceElements = [];
				var sentenceElementsByNamedEntityID = {};

				for(var i in data)
				{
					var sentence = data[i].tokens;
					if(sentence.length === 0)
						continue;

					var sentenceElement = $('<div class="coref-sentence"></div>').appendTo(viewElement);
					allSentenceElements.push(sentenceElement);

					var prev;
					var quotes = {};

					for(var j in sentence)
					{
						var token = sentence[j];
						var tokenClasses = [];

						var idBadge = '';

						if(token.namedEntityID)
						{
							var namedEntityTypeIndex = 0;
							if(token.namedEntityType)
							{
								namedEntityTypeIndex = namedEntityTypes[token.namedEntityType];
								// console.log(namedEntityTypeIndex);
								if(namedEntityTypeIndex === undefined)
								{
									namedEntityTypes[token.namedEntityType] = nextNamedEntityTypeIndex;
									namedEntityTypeIndex = nextNamedEntityTypeIndex;
									nextNamedEntityTypeIndex += 1;
								}
							}

							tokenClasses.push('token named-entity');
							tokenClasses.push('type-'+namedEntityTypeIndex);

							// console.log(namedEntityTypeIndex);
							
							idBadge = '<span class="id-badge">'+token.namedEntityID+'</span>';
						}

						if(tokenClasses.length > 0)
							tokenClasses = ' '+tokenClasses.join(' ');
						else
							tokenClasses = '';

						
						// insert space
						var g = gap(prev, token, quotes);
						if(g !== '') sentenceElement.append(g);

						var entityID = '';
						if(token.namedEntityID)
							entityID = ' data-named-entity-id="'+token.namedEntityID+'"';

						var template = '<span class="token'+tokenClasses+'" data-sentence-index="'+i+'" data-token-index="'
							+j+'"'+entityID+'>'+token.word+idBadge+'</span>';

						var tokenElement = $(template).appendTo(sentenceElement);

						// TODO: saglabāt vajag jquery elementus, vai labāk tikai DOMElementus ?

						allTokenElements.push(tokenElement[0]);

						if(token.namedEntityID)
						{
							tokenElement.click(function (token, event) {
								event.stopPropagation();
								// var namedEntityID = token.namedEntityID;
								// console.log('entity id:', namedEntityID);
								// console.log('token:', token);
								selectNamedEntitiesWithID(token.namedEntityID);
								// NOTE: bind šeit ir būtiski, citādi token nav pareizs
							}.bind(this, token));


							var namedEntities = namedEntityElementsByID[token.namedEntityID];
							if(namedEntities === undefined)
								namedEntities = namedEntityElementsByID[token.namedEntityID] = [];
							namedEntities.push(tokenElement);

							var namedEntitySentenceElements = sentenceElementsByNamedEntityID[token.namedEntityID]; 
							if(namedEntitySentenceElements === undefined)
								namedEntitySentenceElements = sentenceElementsByNamedEntityID[token.namedEntityID] = [];
							namedEntitySentenceElements.push(sentenceElement);
						}
						else
						{
							// deselect named entities
							// TODO: select token
							tokenElement.click(function (event) {
								event.stopPropagation();
								selectNamedEntitiesWithID();
							});
						}

						prev = token;
					}
				}

				for(var type in namedEntityTypes)
				{
					if(type === '_')
						type = 'NONE';

					var typeIndex = namedEntityTypes[type];

					$('<div class="token named-entity type-'+typeIndex+'">'+type+'</div><br/>').appendTo(sidebarElement);
				}
			}

			scope.$watch(attrs.ngModel, function (value) {
				fill(value);
			});
		}
	};
});

app.controller('AppController', function ($scope, $location, $timeout, $http) {

	var noGapBefore = ['.', ',', ':', ';', '!', '?', ')', ']', '}', '%'];
	var noGapAfter = ['(', '[', '{'];
	var quoteSymbols = ["'", '"'];

	function gap(prev, next, quotes)
	{
		if(!prev && quoteSymbols.indexOf(next.lemma) !== -1)
			quotes[next.lemma] = !quotes[next.lemma];
		if(!prev)
			return '';
		if(noGapAfter.indexOf(prev.lemma) !== -1)
			return '';
		if(noGapBefore.indexOf(next.lemma) !== -1)
			return '';
		if(quoteSymbols.indexOf(next.lemma) !== -1)
		{
			var quoted = quotes[next.lemma];
			quotes[next.lemma] = !quotes[next.lemma];
			if(quoted)
				return '';
		}
		if(quoteSymbols.indexOf(prev.lemma) !== -1)
		{
			if(quotes[prev.lemma])
				return '';
		}
		return ' ';
	}

	$scope.extractText = function (tokens) {

		var text = '';
		var prev;
		var quotes = {};

		for(var i in tokens)
		{
			var token = tokens[i];
			// insert space
			text += gap(prev, token, quotes);
			text += token.word;
			prev = token;
		}

		return text;
	};

	$scope.parseCoNLL = function (conll, callback) {

		if(!conll || conll.length == 0)
			return;

		// var lines = conll.match(/[^\r\n]+/g);
		var lines = conll.split(/\n/);

		var sentences = [];
		var sentence = [];

		for(var i in lines)
		{
			var line = lines[i].trim();
			if(line.length == 0)
			{
				if(sentence.length > 0)
				{
					if(callback)
						callback(sentence);
					sentences.push({tokens: sentence, text: $scope.extractText(sentence), inProgress: false});
				}
				sentence = [];
				continue;
			}


			var parts = line.split('\t');
			var namedEntityIDIndex = 10;
			var namedEntityTypeIndex = 11;
			var frameElementsIndex = 12;
			if(parts.length > 12)
			{
				namedEntityIDIndex = 12;
				namedEntityTypeIndex = 13;
				frameElementsIndex = 14;
			}
			else if(parts.length < 13)
			{
				namedEntityIDIndex = 7;
				namedEntityTypeIndex = 8;
				frameElementsIndex = 9;
			}
			sentence.push({
				index: parseInt(parts[0]),
				word: parts[1],
				lemma: parts[2],
				tag0: parts[3],
				tag: parts[4],
				features: parts[5],
				parentIndex: parseInt(parts[6] && parts[6].length > 0 ? parseInt(parts[6]) : -1),
				// namedEntityID: parts[7] === '_' ? undefined : parts[7],
				// namedEntityType: parts[8] === '_' ? undefined : parts[8],
				// frameElements: parts[9] === '_' ? undefined : parts[9],
				// namedEntityID: parts[10] === '_' ? undefined : parts[10],
				// namedEntityType: parts[11] === '_' ? undefined : parts[11],
				// frameElements: parts[12] === '_' ? undefined : parts[12],
				namedEntityID: parts[namedEntityIDIndex] === '_' ? undefined : parts[namedEntityIDIndex],
				namedEntityType: parts[namedEntityTypeIndex] === '_' ? undefined : parts[namedEntityTypeIndex],
				frameElements: parts[frameElementsIndex] === '_' ? undefined : parts[frameElementsIndex],
				all: parts
			});
		}
		return sentences;
	};

	$scope.global = {
		data: {
			conll: undefined
		}
	};

	$scope.setCoNLL = function (text) {
		$scope.global.data.conll = $scope.parseCoNLL(text);
		return;
		// if($scope.global.data.conll === undefined)
		// 	$scope.global.data.conll = [];
		// $scope.global.data.conll.length = 0;
		// var newCoNLL = $scope.parseCoNLL(text);
		// for(var i in newCoNLL)
		// {
		// 	$scope.global.data.conll.push(newCoNLL[i]);
		// }
	};

});

app.controller('CoNLLController', function ($scope, $location, $timeout, $http) {

	function generateSentenceCoNLL(sentence) {
		var result = '';
		for(var i in sentence.tokens)
		{
			var token = sentence.tokens[i];
			var parts = [token.index, token.word, token.lemma, token.tag0, token.tag, token.features, token.parentIndex, '_', '_', '_'];
			// var parts = [token.index, token.word, token.lemma, token.tag0, token.tag, token.features, token.parentIndex];
			parts.push(token.namedEntityID ? token.namedEntityID : '_');
			parts.push(token.namedEntityType ? token.namedEntityType : '_');
			parts.push(token.frameElements ? token.frameElements : '_');
			result += parts.join('\t') + '\n';
		}
		return result;
	};

	$scope.downloadCoNLL = function () {
		var conll = [];
		for(var i in $scope.data.conll)
		{
			conll.push(generateSentenceCoNLL($scope.data.conll[i]));
		}
		showSave(conll.join('\n'), 'output.conll', 'application/json');
	};

	$scope.selectedTabs = { all: false, edit: true, tree: false };

	$scope.state = {
		inProgress: false,
		singleParseInProgress: false,
		parseAllInProgress: false,
		parseAllProgress: 0
	};

	$scope.selected = undefined;
	$scope.select = function (sentence) {
		$scope.selected = sentence;
	};
	$scope.data = $scope.global.data;

	$scope.parseAll = function (event) {

		$scope.state.inProgress = true;
		$scope.state.parseAllInProgress = true;
		var i = 0;

		function next() {
			if(i >= $scope.data.conll.length)
			{
				$scope.state.inProgress = false;
				$scope.state.parseAllInProgress = false;
			}
			else
			{
				$scope.data.conll[i].inProgress = true;
				$http.post('rest/parse', generateSentenceCoNLL($scope.data.conll[i])).success(onSuccess.bind(this, i)).error(onError.bind(this, i));
			}
			i += 1;
		}

		function onSuccess(index, data, status, headers, config) {
			var conll = $scope.parseCoNLL(data)[0];
			$scope.data.conll[index].tokens = conll.tokens;
			$scope.data.conll[index].text = $scope.extractText(conll.tokens);
			$scope.data.conll[index].inProgress = false;
			$scope.state.parseAllProgress = (index/$scope.data.conll.length).toFixed(2).toString();
			// $scope.state.parseAllProgress = index/$scope.data.conll.length;
			next();
		}

		function onError(index, data, status, headers, config) {
			$scope.data.conll[index].inProgress = false;
			$scope.state.parseAllProgress = index/$scope.data.conll.length;
			console.log('error:', data);
			next();
		}

		next();
	};

	// $scope.parseAll2 = function (event) {
	// 	$scope.state.inProgress = true;
	// 	var conll = [];
	// 	for(var i in $scope.data.conll)
	// 	{
	// 		conll.push(generateSentenceCoNLL($scope.data.conll[i]));
	// 	}
	// 	conll = conll.join('\n');
	// 	// console.log('will send:', conll);
	// 	$http.post('rest/parse2', conll).success(function (data, status, headers, config) {
	// 		$scope.state.inProgress = false;
	// 		console.log('success:', data);
	// 		$scope.setCoNLL(data);
	// 	}).error(function (data, status, headers, config) {
	// 		$scope.state.inProgress = false;
	// 		console.log('error:', data);
	// 	});
	// };

	$scope.parse = function () {
		$scope.state.singleParseInProgress = true;
		$scope.selected.inProgress = true;
		var conll = generateSentenceCoNLL($scope.selected);
		// console.log(conll);
		$http.post('rest/parse', conll).success(function (data, status, headers, config) {
			// console.log('success:', data);
			var conll = $scope.parseCoNLL(data)[0];
			$scope.selected.tokens = conll.tokens;
			$scope.selected.text = $scope.extractText(conll.tokens);
			$scope.selected.inProgress = false;
			$scope.state.singleParseInProgress = false;
		}).error(function (data, status, headers, config) {
			$scope.selected.inProgress = false;
			$scope.state.singleParseInProgress = false;
			console.log('error:', data);
		});
	};

	$scope.resolveCorefs = function() {
		$scope.state.inProgress = true;
		var conll = [];
		for(var i in $scope.data.conll)
		{
			conll.push(generateSentenceCoNLL($scope.data.conll[i]));
		}
		conll = conll.join('\n')+'\n';
		// console.log(conll);
		$http.post('rest/coref', conll).success(function (data, status, headers, config) {
			// console.log('success:', data);
			$scope.setCoNLL(data);
			$scope.state.inProgress = false;
		}).error(function (data, status, headers, config) {
			$scope.state.inProgress = false;
			console.log('error:', data);
		});
	};

	$scope.dropFile = function (event) {
		var files = event.dataTransfer.files;
		if(files.length > 0)
		{
			for (var i = 0, f; f = files[i]; i++)
			{
				var reader = new FileReader();
				reader.onload = function (event) {
					$scope.$apply(function () {
						var text = this.result;
						$scope.setCoNLL(text);
						$scope.selected = undefined;
						// $scope.data.text = this.result;
					}.bind(this));
				};

				reader.readAsText(f);
			}
		}
	};

	$scope.$watch('selectedTabs.tree', function (value) {
		if(!value) return;
		if(!$scope.selected)
		{
			setTree(createTree());
			return;
		}
		$scope.selected.json = createTree($scope.selected.tokens);
		setTree($scope.selected.json);
	});

	// TODO: tree watch $scope.selected and update
	// TODO: tree as directive...
	$scope.$watch('selected', function (value) {
		if($scope.selectedTabs.tree)
		{
			if(!$scope.selected)
			{
				setTree(createTree());
				return;
			}
			$scope.selected.json = createTree($scope.selected.tokens);
			setTree($scope.selected.json);
		}
		// console.log('selected change:', value);
	});

	$scope.$watch('selected.tokens', function (value) {
		if(!value)
			return;
		$scope.selected.text = $scope.extractText(value);
	}, true);


	// tree view
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

		if(!data)
			return root;

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
			injectInto: 'treeView',
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


app.controller('TextInputController', function ($scope, $location, $timeout, $http) {

	$scope.data = {
		text: ''
	};
	$scope.state = {
		inProgress: false,
		splitInProgress: false
	};

	$scope.dropTextFile = function (event) {
		var files = event.dataTransfer.files;
		if(files.length > 0)
		{
			for (var i = 0, f; f = files[i]; i++)
			{
				var reader = new FileReader();
				reader.onload = function (event) {
					$scope.$apply(function () {
						$scope.data.text = this.result;
					}.bind(this));
				};

				reader.readAsText(f);
			}
		}
	};

	$scope.dropTextFileAndExtract = function (event) {
		var files = event.dataTransfer.files;
		if(files.length > 0)
		{
			for (var i = 0, f; f = files[i]; i++)
			{
				var reader = new FileReader();
				reader.onload = function (event) {
					$scope.$apply(function () {
						var text = this.result;
						$scope.extractSentences(text);
						// $scope.data.text = this.result;
					}.bind(this));
				};

				reader.readAsText(f);
			}
		}
	};

	$scope.exportToCoNLL = function () {
		$scope.state.inProgress = true;

				// console.log('will send:', $scope.data.sentences);
				// return;
		// for(var i in $scope.data.sentences)
		// {
		// 	var sentence = $scope.data.sentences[i];
			// $http.post('rest/conllize', $scope.sentence).success(function (data, status, headers, config) {
			$http.post('rest/conllize2', $scope.data.sentences.join('\n')).success(function (data, status, headers, config) {
				$scope.state.inProgress = false;
				// console.log('success:', data);
				$scope.setCoNLL(data);
			}).error(function (data, status, headers, config) {
				$scope.state.inProgress = false;
				console.log('error:', data);
			});
		// }

		// $scope.state.inProgress = false;
	};

	$scope.extractSentences = function (text) {
		$scope.state.splitInProgress = true;
		$http.post('rest/split', text).success(function (data, status, headers, config) {
			// console.log('success:', data);
			// $scope.data.sentences = data;
			$scope.data.sentences = [];
			var sentences = data.trim().split(/\n/);
			for(var i in sentences)
			{
				var sentence = sentences[i];
				if(sentence.length > 0)
					$scope.data.sentences.push(sentence);
			}
			// $scope.data.sentences = data.trim().split(/\n/);
			$scope.state.splitInProgress = false;
		}).error(function (data, status, headers, config) {
			$scope.state.splitInProgress = false;
			console.log('error:', data);
		});
	}
});

app.controller('SentencesController', function ($scope, $location, $timeout, $http) {

	$scope.selected = undefined;
	$scope.select = function (index) {
		$scope.selected = index;
	};

	$scope.remove = function () {
		if($scope.selected === undefined)
			return;

		$scope.data.sentences.splice($scope.selected, 1);

		if($scope.data.sentences.length === 0)
			$scope.selected = undefined;
		else if($scope.selected >= $scope.data.sentences.length)
			$scope.selected = $scope.data.sentences.length-1;
	};

	$scope.addNew = function () {
		$scope.data.sentences.push('Jauns teikums.');
		$scope.selected = $scope.data.sentences.length-1;
		// TODO: scroll into view ar element.scrollIntoView(false), bet vajag element
	};

});

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

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

