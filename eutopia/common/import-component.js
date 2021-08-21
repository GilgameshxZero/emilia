// Dynamic component loading.
export function importComponent(name) {
	const parser = new DOMParser();
	return new Promise((resolve, reject) => {
		fetch(`${name}/${name}.html`).then((res) => {
			return res.text();
		}).then((text) => {
			console.log(text);
			customElements.define(`eutopia-${name}`, class extends HTMLElement {
				constructor() {
					super();
					this.attachShadow({ mode: `open` }).appendChild(parser.parseFromString(text, "text/html").querySelector(`template`).content.cloneNode(true));
					resolve(this);
				}
			});
		}).catch((reason) => {
			reject(reason);
		});
	});
}
