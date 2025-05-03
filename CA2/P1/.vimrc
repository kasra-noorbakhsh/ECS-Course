nnoremap <leader>w :w<CR>
nnoremap <leader>v :vsp<CR>
nnoremap <leader>h :sp<CR>
nnoremap <leader>n :NERDTreeToggle<CR>
set number
set ignorecase
set hlsearch
set tabstop=4
set shiftwidth=4
set expandtab
syntax enable
colorscheme desert
" Enable YouCompleteMe
let g:ycm_auto_trigger = 1
let g:ycm_min_num_of_chars_for_completion = 2

call plug#begin('~/.vim/plugged')
Plug 'ycm-core/YouCompleteMe'
Plug 'preservim/nerdtree'
call plug#end()
