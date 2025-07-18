# typed: true

class Parent
  extend T::Sig

  sig {overridable.returns(Integer)}
  def some_public_api; 0; end
end

class ChildBad < Parent
  sig {override.returns(Integer)}
  private def some_public_api; 1; end # error: Method `some_public_api` is private in `ChildBad` but not in `Parent`
end

class ChildOkay < Parent
  sig {override(allow_incompatible: :visibility).returns(Integer)}
  private def some_public_api; 1; end
end

class ChildBadTypes < Parent
  sig {override(allow_incompatible: :visibility).returns(String)}
  private def some_public_api; ''; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Return type `String` does not match return type of overridable method `Parent#some_public_api`
end

class ChildBadSymbol < Parent
  sig {override(allow_incompatible: :bad).returns(Integer)}
  #                                 ^^^^ error: `override(allow_incompatible: ...)` expects either `true` or `:visibility
  private def some_public_api; 0; end
  #       ^^^^^^^^^^^^^^^^^^^ error: Method `some_public_api` is private in `ChildBadSymbol` but not in `Parent`
end

class ChildBoth1 < Parent
  sig { override(allow_incompatible: true).override(allow_incompatible: :visibility).returns(Integer) }
  #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Don't use both `override(allow_incompatible: true)` and `override(allow_incompatible: :visibility)
  private def some_public_api; 0; end
end

class ChildBoth2 < Parent
  sig { override(allow_incompatible: :visibility).override(allow_incompatible: true).returns(Integer) }
  #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Don't use both `override(allow_incompatible: true)` and `override(allow_incompatible: :visibility)
  private def some_public_api; 0; end
end

class ChildBoth3 < Parent
  sig { returns(Integer).override(allow_incompatible: :visibility).override(allow_incompatible: true) }
  #                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: Don't use both `override(allow_incompatible: true)` and `override(allow_incompatible: :visibility)
  private def some_public_api; 0; end
end

class ChildBoth4 < Parent
  sig { override(allow_incompatible: false).override(allow_incompatible: :visibility).returns(Integer) }
  #                                  ^^^^^  error: `override(allow_incompatible: ...)` expects either `true` or `:visibility`
  private def some_public_api; 0; end
end

class ChildBoth5 < Parent
  # TODO(jez) Probably want to ban this, but that's probably something to leave until
  # https://github.com/sorbet/sorbet/issues/9012
  sig { override.override(allow_incompatible: :visibility).returns(Integer) }
  private def some_public_api; 0; end
end

