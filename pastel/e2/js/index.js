let data;

window.onload = function () {
	let body = document.getElementsByTagName("body")[0];

	loadDB(function (db) {
		data = db;
		let metadata = data["metadata"];
		let tables = data["tables"];

		//build the display for the tables
		for (let table in tables) {
			let tableRef = tables[table];
			let tableElem = document.createElement("div");
			tableElem.classList.add("table");
			tableElem.title = table;
			body.appendChild(tableElem);

			//title
			let titleElem = document.createElement("div");
			titleElem.classList.add("title");
			tableElem.appendChild(titleElem);
			titleElem.innerHTML = table;

			for (let row = 0; row < tableRef.length; row++) {
				let rowRef = tableRef[row];
				let rowElem = document.createElement("div");
				rowElem.classList.add("row");
				rowElem.ind = row;
				tableElem.appendChild(rowElem);

				for (let cell = 0; cell < rowRef.length; cell++) {
					let cellRef = rowRef[cell];
					let cellElem = document.createElement("div");
					cellElem.classList.add("cell");
					cellElem.ind = cell;
					rowElem.appendChild(cellElem);

					//set the width of this cell using flex-grow
					cellElem.style.flexGrow = metadata["column-width"][table][cell];

					if (cellRef[0] == "set") {
						cellElem.classList.add("cell-set");

						for (let setElem = 0; setElem < cellRef[1].length; setElem++) {
							let setElemElem = document.createElement("div");
							setElemElem.classList.add("cell-set-elem");
							setElemElem.ind = setElem;
							setElemElem.addEventListener("click", onClickSetElem);
							cellElem.appendChild(setElemElem);

							cellRef[1][setElem].push(setElemElem);
							if (cellRef[1][setElem][0] == "formula") {
								setElemElem.classList.add("formula");
								setElemElem.formula = cellRef[1][setElem][1];
								computeFormulaElem(setElemElem);
							} else {
								setElemElem.innerHTML = cellRef[1][setElem][1];
							}
						}
					} else {
						cellRef.push(cellElem);
						if (cellRef[0] == "formula") {
							cellElem.classList.add("formula");
							cellElem.formula = cellRef[1];
							computeFormulaElem(cellElem);
						} else {
							cellElem.innerHTML = cellRef[1];
						}
					}
				}
			}
		}
	})
}

//dummy function which should link into data back-end
function loadDB(callback) {
	let xhr = new XMLHttpRequest();
	xhr.open("GET", "assets/db.json", true);
	xhr.responseType = "json";
	xhr.onload = function () {
		var status = xhr.status;
		if (status === 200) {
			callback(xhr.response);
		}
	};
	xhr.send();
}

function computeFormulaElem(elem, dependsOnElem) {
	let expression = "." + elem.formula;
	let context = data["tables"];

	//evaluate this
	let thisRow = elem;
	while (!thisRow.classList.contains("row")) {
		thisRow = thisRow.parentNode;
	}
	let thisExp = thisRow.parentNode.title + "[" + thisRow.ind + "]";
	expression = expression.replace("this", thisExp);

	//evaluate all focuses in expression
	//limit to 1 focus chain for now, but can daisy chain with columns
	let invalid = false;
	let focusSplit = expression.split(".focus");
	for (let a = 0; a < focusSplit.length - 1; a++) {
		let xElem = eval("context" + focusSplit[a]);
		console.log(xElem);
		if (typeof xElem[1] === "string" && !xElem[2].classList.contains("cell-set")) {
			invalid = true;
			break;
		}

		let setElem;
		if (typeof xElem[1] === "string") {
			setElem = xElem[2];
		} else {
			setElem = eval("context" + focusSplit[a])[1][0][2].parentNode;
		}
		if (dependsOnElem !== undefined && setElem !== dependsOnElem) {
			//don't modify this focus element if it hasn't changed
			return;
		}

		let focusElem = setElem.getElementsByClassName("cell-set-elem-focus")[0];
		if (focusElem === undefined) {
			invalid = true;
			break;
		}

		//remove the last brackets
		let focusFormula = focusElem.formula;
		focusFormula = focusFormula.substr(0, focusFormula.lastIndexOf("["));
		let focusData = eval("context." + focusFormula);

		//renew context
		context = focusData;
		//console.log("Focus context: " + context);
	}

	if (!invalid) {
		expression = focusSplit[focusSplit.length - 1];
		let innerHTML = eval("context" + expression)[1];

		if (innerHTML instanceof Array) {
			elem.innerHTML = "";
			elem.classList.add("cell-set");

			for (let setElem = 0; setElem < innerHTML.length; setElem++) {
				let setElemElem = document.createElement("div");
				setElemElem.classList.add("cell-set-elem");
				setElemElem.ind = setElem;
				setElemElem.addEventListener("click", onClickSetElem);
				elem.appendChild(setElemElem);

				if (innerHTML[setElem][0] == "formula") {
					setElemElem.classList.add("formula");
					setElemElem.formula = innerHTML[setElem][1];
					computeFormulaElem(setElemElem);
				} else {
					setElemElem.innerHTML = innerHTML[setElem][1];
				}

				innerHTML[setElem].push(setElemElem);
			}
		} else {
			elem.innerHTML = innerHTML;
		}
	} else {
		elem.innerHTML = "no focus";
	}
}

function onClickSetElem() {
	console.log(this);
	if (this.classList.contains("cell-set-elem-focus")) {
		this.classList.remove("cell-set-elem-focus");
	} else {
		for (let a = 0; a < this.parentNode.children.length; a++) {
			this.parentNode.children[a].classList.remove("cell-set-elem-focus");
		}

		this.classList.add("cell-set-elem-focus");
	}

	//for now, just re-evaluate all formulas
	let formulaElems = document.getElementsByClassName("formula");
	for (let a = formulaElems.length - 1; a >= 0; a--) {
		computeFormulaElem(formulaElems[a], this.parentNode);
	}
}