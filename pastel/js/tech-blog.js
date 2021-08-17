var listSelector;
var listViewer;

var curMListSelect = null;

function fetchText(url) {
	return fetch(url).then(function (response) {
		return response.text();
	});
}

function onLoad() {
	listSelector = document.getElementsByClassName("list-selector")[0];
	listViewer = document.getElementsByClassName("list-viewer")[0];

	let prevColor = 1;
	for (let a = 0; a < listSelector.children.length; a++) {
		listSelector.children[a].addEventListener("click", onSelectorClick, false);

		//dynamically assign titles based on fetched text
		let titleNode = document.createElement("div");
		let indexNode = document.createElement("div");
		let pageNode = document.createElement("div");
		listSelector.children[a].appendChild(titleNode);
		listSelector.children[a].appendChild(indexNode);
		listSelector.children[a].appendChild(pageNode);

		titleNode.className = "list-selector-panel-title";
		indexNode.className = "list-selector-panel-index";
		pageNode.className = "list-page";

		fetchText("tech-blog/" + listSelector.children[a].id + ".html").then(function (html) {
			pageNode.innerHTML = html;

			let title = pageNode.getElementsByTagName("h1")[0].textContent;
			let subtitle = pageNode.getElementsByTagName("p")[0].textContent;
			titleNode.appendChild(document.createElement("p"));
			titleNode.children[0].innerHTML = "<b>" + title + "</b><br>" + subtitle;

			let imgs = pageNode.getElementsByTagName("img");
			for (let b = 0; b < imgs.length; b++) {
				let spanChild = document.createElement("span");
				imgs[b].parentNode.insertBefore(spanChild, imgs[b]);
				spanChild.className = "image center full-width";
				spanChild.appendChild(imgs[b]);

				let imgSplit = imgs[b].src.split("/assets/", 2)
				imgs[b].src = imgSplit[0] + "/tech-blog/assets/" + imgSplit[1];
			}

			let links = pageNode.getElementsByTagName("a");
			for (let b = 0; b < links.length; b++) {
				let outer = links[b].outerHTML;
				if (outer.split("<a href=\"", 2)[1].slice(0, 4) != "http") {
					let outerSplit = outer.split("assets/", 2);
					links[b].outerHTML = outerSplit[0] + "tech-blog/assets/" + outerSplit[1];
				}
			}
		});

		indexNode.textContent = a + 1;

		//assign color not equal to previous
		let color = (prevColor + 1) % 9 + 1;
		listSelector.children[a].classList.add("color-" + color);
		prevColor = color;
	}

	let fragment = window.location.hash.substr(1);
	let fSplit = fragment.split("&", 2);
	if (fSplit.length >= 2)
		fragment = fSplit[1];
	else
		fragment = listSelector.children[0].id;
	if (document.getElementById(fragment) == null) {
		fragment = listSelector.children[0].id;
	}
	setSelector(fragment);

	listSelector.parentNode.style.opacity = 1;
}
onLoad();

function setSelector(id) {
	//update viewer content
	if (curMListSelect != null) {
		document.getElementById(curMListSelect).appendChild(listViewer.children[0]);
	}
	listViewer.appendChild(document.getElementById(id).getElementsByClassName("list-page")[0]);
	listViewer.className = document.getElementById(id).className;
	listViewer.classList.remove("list-selector-panel");
	listViewer.classList.add("list-viewer");

	listViewer.scrollTop = 0;

	//classes
	if (curMListSelect != null) {
		document.getElementById(curMListSelect).classList.remove("list-selector-panel-selected");
	}
	document.getElementById(id).classList.add("list-selector-panel-selected");

	curMListSelect = id;

	parent.location.hash = window.location.hash.substr(1).split("&", 2)[0] + "&" + id;
}

function onSelectorClick() {
	if (this.classList.contains("list-selector-panel-selected"))
		return;

	setSelector(this.id);
}
