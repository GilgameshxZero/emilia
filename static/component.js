// Dynamic component loading.
//
// Once a component instance is constructed, componentLoad will be set. Once that promise is fulfilled, the shadowDom and component DOM is constructed (including all child components).
//
// The callback onComponentDOMContentLoaded is called when componentLoad and shadowDom exists, but child components may not have loaded yet.
//
// This function may only be called once per unique name.
export function registerComponent(
	name,
	Component = class extends HTMLElement {}
) {
	const fetchComponentHtml = fetch(`components/${name}.html`).then((res) => {
		return res.text();
	});

	customElements.define(
		`emilia-${name}`,
		class extends Component {
			constructor() {
				// Neither onload events nor mutation observers can detect when a component is DOMRectReadOnly. Instead, we query link/script tags and add an event listener to each: <https://stackoverflow.com/questions/45545456/event-when-web-component-has-loaded>.
				super();
				this.componentLoad = new Promise((resolve, reject) => {
					fetchComponentHtml.then((text) => {
						const fragmentClone = new DOMParser()
							.parseFromString(text, "text/html")
							.querySelector(`template`)
							.content.cloneNode(true);

						// When cLoadingResources reaches 0, the component has loaded its external resources.
						let cLoadingResources = 0;
						const onLoadResource = () => {
							cLoadingResources--;
							if (cLoadingResources === 0) {
								// Component instance ready (as soon as available fonts are loaded)!
								document.fonts.ready.then(() => {
									resolve(this);
								});
							}
						};

						// Add resource event listeners before loading is triggered.
						prependRelativePaths(fragmentClone, `components/`);
						fragmentClone.querySelectorAll(`link`).forEach((link) => {
							// Only links with href can be loaded.
							if (link.hasAttribute(`href`)) {
								cLoadingResources++;
								link.addEventListener(`load`, onLoadResource);
							}
						});
						fragmentClone.querySelectorAll(`script`).forEach((script) => {
							// Only scripts with src can be loaded.
							if (script.hasAttribute(`src`)) {
								cLoadingResources++;
								script.addEventListener(`load`, onLoadResource);
							}
						});
						// If document has no external resources, resolve just with fonts.
						if (cLoadingResources === 0) {
							document.fonts.ready.then(() => {
								resolve(this);
							});
						}

						// Loading occurs once the shadowRoot is attached. Perform any custom DOM setup here.
						this.attachShadow({ mode: `open` }).appendChild(fragmentClone);
						if (typeof this.onComponentDOMContentLoaded === `function`) {
							this.onComponentDOMContentLoaded();
						}
					});
				});
			}
		}
	);
}

// Prepend a prefix to all relative resource paths under given element, and resolves to an absolute URL. Respects the <base> element.
export function prependRelativePaths(element, prefix) {
	const baseURL = new URL(document.baseURI);
	[`href`, `src`].forEach((attribute) => {
		element.querySelectorAll(`[${attribute}]`).forEach((element) => {
			const value = element.getAttribute(attribute);
			try {
				// An absolute URL with origin should succeed here.
				new URL(value);
			} catch (exception) {
				let proposition = prefix + element.getAttribute(attribute);
				// Exceptions thrown here are true errors.
				element.setAttribute(
					attribute,
					`${new URL(proposition, baseURL.origin + baseURL.pathname).pathname}`
				);
			}
		});
	});
}
