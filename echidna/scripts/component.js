// Dynamic component loading. May only be called once per unique name.
//
// Component lifecycle:
// 1. onDomLoad: Called when the component's shadowDom is constructed, but child components may not have loaded yet.
// 2. resourceLoad: All external links and scripts in the shadowDom have been loaded.
// 3. subcomponentLoad: Most components implement this, which is resolved when all child components have subcomponentLoaded. This may be reset when the component is reset, with the meaning of reset differing between components.
export function registerComponent(
	name,
	storyworld = ``,
	Component = class extends HTMLElement {}
) {
	const fetchComponentHtml = fetch(
		`${storyworld}/components/${name}.html`
	).then((res) => {
		return res.text();
	});

	customElements.define(
		`emilia-${name}`,
		class extends Component {
			constructor() {
				super();

				// Neither onload events nor mutation observers can detect when a component is DOMRectReadOnly. Instead, we query link/script tags and add an event listener to each: <https://stackoverflow.com/questions/45545456/event-when-web-component-has-loaded>.
				this.resourceLoad = new Promise((resolve) => {
					// All components start out invisible, and are only unblocked from being so at resourceLoad, which allows time for the element to react with any CSS files it has linked.
					const invisibleStyle = document.createElement(`style`);
					invisibleStyle.innerHTML = `:host { opacity: 0 !important; }`;

					// Waits for fonts, then unblocks opacity and resolves.
					const resolveResourceLoad = () => {
						document.fonts.ready.then(() => {
							this.shadowRoot.removeChild(invisibleStyle);
							resolve();
						});
					};

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
								resolveResourceLoad();
							}
						};

						// Add resource event listeners before loading is triggered.
						prependRelativePaths(fragmentClone, `${storyworld}/components/`);
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

						// Loading occurs once the shadowRoot is attached. Perform any custom DOM setup here.
						this.attachShadow({ mode: `open` });
						this.shadowRoot.appendChild(invisibleStyle);
						this.shadowRoot.appendChild(fragmentClone);
						if (typeof this.onDomLoad === `function`) {
							this.onDomLoad();
						}

						// If document has no external resources, resolve just with fonts, but only after onDomLoad has been called.
						if (cLoadingResources === 0) {
							resolveResourceLoad();
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
