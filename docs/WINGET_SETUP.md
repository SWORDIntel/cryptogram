# WinGet Publishing Setup for CRYPTOGRAM

## What is WinGet?
WinGet is the official Windows Package Manager. Publishing CRYPTOGRAM to WinGet allows users to install it with:
```
winget install SWORDOps.CRYPTOGRAM
```

## How to Enable WinGet Publishing

### Step 1: Create a Personal Access Token (PAT)

1. Go to GitHub Settings: https://github.com/settings/tokens
2. Click **"Generate new token"** → **"Generate new token (classic)"**
3. Give it a name like `CRYPTOGRAM-WinGet-Publisher`
4. Set expiration (recommend: 1 year)
5. Select scopes:
   - ✅ `public_repo` (for public repositories)
   - ✅ `workflow` (to update workflows)
6. Click **"Generate token"**
7. **COPY THE TOKEN** - you won't see it again!

### Step 2: Add Token to Repository Secrets

1. Go to your repository: https://github.com/SWORDOps/CRYPTOGRAM
2. Click **Settings** → **Secrets and variables** → **Actions**
3. Click **"New repository secret"**
4. Name: `WINGET_TOKEN`
5. Value: Paste the token you copied
6. Click **"Add secret"**

### Step 3: How It Works

Once the secret is added, the WinGet workflow will automatically:
- Trigger when you create a GitHub Release (tagged with `v*`)
- Submit CRYPTOGRAM to the Windows Package Manager Community Repository
- Create/update the WinGet manifest automatically

### Step 4: Create Your First Release (Test)

```bash
# Tag a release
git tag v1.0.0
git push origin v1.0.0

# Or create release via GitHub UI at:
# https://github.com/SWORDOps/CRYPTOGRAM/releases/new
```

### Important Notes

1. **First submission**: The first time you publish, WinGet maintainers will review the manifest. This may take 1-3 days.

2. **Subsequent updates**: After initial approval, updates are usually automatic.

3. **Package identifier**: The workflow is configured with identifier `SWORDOps.CRYPTOGRAM`

4. **Requirements**:
   - Release must include a Windows `.exe` installer
   - Release must be tagged (e.g., `v1.0.0`)
   - Release must not be a draft or pre-release (unless configured)

## Troubleshooting

**Issue**: Token doesn't work
- Verify the token has `public_repo` and `workflow` scopes
- Check token hasn't expired
- Ensure token is added to secrets as `WINGET_TOKEN`

**Issue**: No installer found
- Ensure your release includes a `.exe` file
- Check the Windows build workflow completed successfully

**Issue**: Manifest rejected
- Check WinGet community repo for PR feedback
- Common issues: incorrect version format, missing installer hash

## Disable WinGet Publishing

If you want to disable it later, you can:
1. Delete the `WINGET_TOKEN` secret, OR
2. Delete/disable the `.github/workflows/winget.yml` file

## Additional Resources

- WinGet Documentation: https://learn.microsoft.com/en-us/windows/package-manager/
- WinGet Community Repository: https://github.com/microsoft/winget-pkgs
- WinGet Releaser Action: https://github.com/vedantmgoyal2009/winget-releaser
