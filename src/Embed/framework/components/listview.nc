<component name="listview">
    <nss>
        list: {
            backgroundColor: "#f8f8f8",
            flexDirection: "column",
            flexGrow: 1,
            color: "#d0d0d0",
            fontSize: 16,
            lineHeight: 56,
            textAlign: "left",
            overflow: false
        },

        icon: {
            flexGrow: 1,
            fontSize: 24,
            lineHeight: 24,
            width: 24,
            maxWidth: 24
        },

        media: {
            flexGrow: 1,
            color: "#d0d0d0",
            textAlign: "center",
            fontSize: 32,
            lineHeight: 50,
            maxWidth: 50,
            minHeight: 50
        },

        chevron: {
            flexGrow: 1,
            color: "#d8d8d8",
            marginRight: 6
        },

        title : {
            flexGrow: 4,
            color: "#444444"
        },

        li : {
            height: 56,
            borderColor: "#d0d0d0",
            borderWidth: 0.50
        }
    </nss>
    <layout class="list">
        <slot></slot>
    </layout>
    <script>
        module.exports = class extends Component {
            constructor(attr) {
                super(attr);

                this.scrollableY = true;
                this.allowNegativeScroll = false;

                this.on("scroll", (e) => {
                   this.infiniteScrollCheck();
                });

                this.reset();
            }

            reset() {
                this.currPage = 0;
                this.items = [];
                this.viewport = [];
            }

            select(id) {
                this.emit("select", id);
            }

            getItemTemplate(item){
                let id = item.id;
                let media = item.media;
                let title = item.title;

                return (`
                    <li id="${id}" class="li" on:mousedown='this.select(${id})'>
                        <icon class="icon media" shape="${media}"></icon>
                        <span class="title">${title}</span>
                        <icon class="icon chevron" shape="fa-chevron-right"></icon>
                    </li>
                `);
            }

            addItem(item) {
                this.shadowRoot.jsScope["this"] = this;
                var node = NML.CreateTree(
                    this.getItemTemplate(item), this, this.shadowRoot
                );
            }

            renderViewport() {
                for (var i=0, l=this.viewport.length; i<l; i++) {
                    let item = this.viewport[i];
                    this.addItem(item);
                }
            }

            setItems(list) {
                this.reset();

                let { width, height } = this.getDimensions();
                let itemHeight = 56;

                this.currItem = 0;
                this.page_count = Math.floor(height/itemHeight);

                for (var i=0, l=list.length; i<l; i++) {
                    let item = list[i];
                    this.items[i] = item;
                    if (i <= (2*this.page_count)) {
                        this.addItem(item);
                        this.currItem = i;
                    }
                }
            }

            infiniteScrollCheck() {
                let { width, height } = this.getDimensions();

                var scale = height / this.innerHeight,
                    maxScrollTop = Math.min(this.scrollTop, this.innerHeight - height);

                /*
                container.style.top = radius;
                container.style.right = radius*0.5;
                container.style.height = height - 2*radius - 0;
                */

                var handleTop = Math.round(maxScrollTop * scale);
                var handleHeight = Math.round(height * scale);

                var percentScroll = 100*(handleTop+handleHeight) / height;

                if (percentScroll>80) {
                    this.loadMore();
                }
            }

            loadMore() {
                this.currItem++;

                var list = this.items,
                    l = list.length,
                    max = Math.min(this.currItem + 2*this.page_count, l);

                // console.log("load more", this.currItem + '->' + max, "items");

                for (var i=this.currItem; i<max; i++) {
                    let item = list[i];
                    this.addItem(item);
                    this.currItem = i;
                }
            }
        }
    </script>
</component>
