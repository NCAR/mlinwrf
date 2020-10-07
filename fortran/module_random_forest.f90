module module_random_forest
    implicit none

    type decision_tree
        integer :: nodes
        integer, allocatable :: node(:)
        integer, allocatable :: feature(:)
        real(kind=8), allocatable :: threshold(:)
        real(kind=8), allocatable :: tvalue(:)
        integer, allocatable :: children_left(:)
        integer, allocatable :: children_right(:)
        real(kind=8), allocatable :: impurity(:)
    end type decision_tree

    type :: RandomForest
       type(decision_tree),allocatable :: trees(:)
    end type RandomForest
contains

    subroutine load_decision_tree(file_path, filename, tree)
        character(len=*), intent(in) :: file_path
        character(len=*), intent(in) :: filename
        type(decision_tree), intent(out) :: tree
        integer :: n, stat
        tree%nodes = 0
        stat = 0
        open(100, file=trim(file_path) // trim(filename), access="sequential", form="formatted")
        read(100, '(A)')
        do
            read(100, *, iostat=stat)
            if (stat < 0) exit
            tree%nodes = tree%nodes + 1
        end do
        rewind(100)
        allocate(tree%node(tree%nodes))
        allocate(tree%feature(tree%nodes))
        allocate(tree%threshold(tree%nodes))
        allocate(tree%tvalue(tree%nodes))
        allocate(tree%children_left(tree%nodes))
        allocate(tree%children_right(tree%nodes))
        allocate(tree%impurity(tree%nodes))
        read(100, '(A)')
        do n=1,tree%nodes
           read(100, *) tree%node(n), tree%feature(n), tree%threshold(n), tree%tvalue(n), &
                   tree%children_left(n), tree%children_right(n), tree%impurity(n)
        end do
        close(100)
    end subroutine load_decision_tree

    subroutine load_random_forest_c(file_path_c_str, file_path_len, random_forest_handle) bind (C)
        use, intrinsic :: iso_c_binding, only:  c_loc, c_f_pointer, c_null_char, c_ptr
        implicit none

        character, dimension(*), intent(in) :: file_path_c_str
        integer, intent(in) :: file_path_len
        type (c_ptr), intent (out) :: random_forest_handle
        type(RandomForest), pointer :: random_forest_ptr
        character(10240), pointer :: file_path
        
        call c_f_pointer (c_loc(file_path_c_str), file_path)

        allocate(random_forest_ptr)
        call load_random_forest(file_path(1:file_path_len), random_forest_ptr%trees)
        random_forest_handle = c_loc(random_forest_ptr)
    end subroutine load_random_forest_c

    subroutine free_random_forest_c (random_forest_handle) bind (C)
        ! free_random_forest_c
        ! Description: function used to free an opaque handle and it's underlying RandomForest pointer object allocated in Fortran.
        !
        ! Input:
        ! random_forest_handle (in): an opaque handle (void* pointer in C) to an array of decision_tree
        !
        use, intrinsic :: iso_c_binding, only:  c_f_pointer, c_ptr
        implicit none

        type (c_ptr), intent (in) :: random_forest_handle
        type(RandomForest), pointer :: random_forest_ptr

        call c_f_pointer(random_forest_handle, random_forest_ptr)
        deallocate(random_forest_ptr)
    end subroutine free_random_forest_c

    subroutine random_forest_predict_c(input_data, in_col_count, in_row_count, random_forest_handle, prediction_data) bind (C)
        use, intrinsic :: iso_c_binding, only:  c_ptr, c_f_pointer
        implicit none

        integer, intent(in) :: in_col_count,in_row_count 
        real(kind=8), intent(in) :: input_data(in_col_count,in_row_count)
        real(kind=8) :: input_data_t(in_row_count,in_col_count)
        type (c_ptr), intent (in) :: random_forest_handle
        type(RandomForest), pointer :: random_forest_ptr
        real(kind=8), intent(out) :: prediction_data(*)
        real(kind=8) :: prediction(in_row_count)

        input_data_t = transpose(input_data)

        call c_f_pointer(random_forest_handle, random_forest_ptr)

        call random_forest_predict(input_data_t,random_forest_ptr%trees,prediction)

        prediction_data(:in_row_count) = prediction
    end subroutine random_forest_predict_c

    subroutine load_random_forest(file_path, random_forest_array)
        character(len=*), intent(in) :: file_path
        type(decision_tree), allocatable, intent(out) :: random_forest_array(:)
        character(len=300), allocatable :: filenames(:)
        integer :: num_trees, n, stat
        num_trees = 0
        stat = 0
        !print *, trim(file_path) // "tree_files.csv"

        open(55, file=trim(file_path) // "tree_files.csv", access="sequential", form="formatted")
        do
            read(55, '(A)', iostat=stat)
            if (stat /= 0) exit
            num_trees = num_trees + 1
        end do
        rewind(55)
        allocate(filenames(num_trees))
        allocate(random_forest_array(num_trees))
        do n=1,num_trees
            read(55, '(A)') filenames(n)
            call load_decision_tree(file_path, trim(filenames(n)), random_forest_array(n))
        end do
        close(55)
        deallocate(filenames)
    end subroutine load_random_forest

    subroutine random_forest_predict(input_data, random_forest_array, prediction)
        real(kind=8), intent(in) :: input_data(:, :)
        type(decision_tree), intent(in) :: random_forest_array(:)
        real(kind=8), intent(out) :: prediction(:)
        integer :: e, t
        integer :: num_trees, num_examples
        real(kind=8), allocatable :: tree_predictions(:)
        num_trees = size(random_forest_array)
        num_examples = size(input_data, 1)
        allocate(tree_predictions(num_trees))
        tree_predictions = 0
        do e=1, size(input_data, 1)
            do t=1, num_trees
                call decision_tree_predict(input_data(e, :), random_forest_array(t), tree_predictions(t))
            end do
            prediction(e) = sum(tree_predictions) / real(num_trees)
        end do
        deallocate(tree_predictions)
    end subroutine random_forest_predict

    subroutine decision_tree_predict(input_data_tree, tree, tree_prediction)
        real(kind=8), intent(in) :: input_data_tree(:)
        type(decision_tree), intent(in) :: tree
        integer :: node
        real(kind=8), intent(out) :: tree_prediction
        logical :: not_leaf
        logical :: exceeds
        node = 1
        tree_prediction = -999
        not_leaf = .TRUE.
        do while (not_leaf)
            if (tree%feature(node) < 0) then
                tree_prediction = tree%tvalue(node)
                not_leaf = .FALSE.
            else
                if (tree%feature(node) + 1 > size(input_data_tree)) then
                    print*, "Node feature number larger than input size", tree%feature(node) + 1, size(input_data_tree)
                end if
                exceeds = input_data_tree(tree%feature(node) + 1) > tree%threshold(node)
                if (exceeds) then
                    node = tree%children_right(node) + 1
                else
                    node = tree%children_left(node) + 1
                end if
            end if
        end do
    end subroutine decision_tree_predict
end module module_random_forest
