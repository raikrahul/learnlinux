# Learn Linux Internals

A deep dive into Linux kernel memory management, page tables, and system internals.

## About

This blog contains detailed, axiomatic tutorials on Linux kernel internals, focusing on:

- **Page Table Mechanics**: CR3, PML4, PDPT, PD, PT structure
- **Memory Addressing**: 4KB, 2MB, 1GB page walks
- **Memory Allocators**: Buddy and Slab allocators
- **Process Memory**: VMAs, Copy-on-Write, Swap
- **TLB/PCID**: Translation caching and optimization

Each tutorial uses numerical derivations with real machine data—no abstract explanations.

## Local Development

```bash
# Install Ruby and Bundler
sudo apt install ruby ruby-dev build-essential

# Install Jekyll and dependencies
bundle install

# Run local server
bundle exec jekyll serve

# Visit http://localhost:4000/learnlinux/
```

## Deploy to GitHub Pages

1. Push to GitHub
2. Go to Settings → Pages
3. Select source branch (master or gh-pages)
4. Site will be at: https://raikrahul.github.io/learnlinux/

## Structure

```
learnlinux/
├── _config.yml          # Jekyll configuration
├── _layouts/            # Page layouts
├── _includes/           # Reusable components
├── _posts/              # Blog posts (YYYY-MM-DD-title.md)
├── _tutorials/          # Tutorial collection
├── _demos/              # Demo documentation
├── assets/              # CSS, JS, images
├── index.md             # Home page
├── Gemfile              # Ruby dependencies
└── README.md            # This file
```

## License

MIT License
